#include <TinyGPSPlus.h>
#include <TimeLib.h>
#include <SD.h>
# define WxSerial Serial2


const byte numChars = 82; // longest possible string from Airmar NMEA 0183
boolean newData = false;
char receivedChars[numChars];
char fileName[] = "windLogger.txt"; //  File Naming
char timeStamp[20]; // time stamp for each packet

const int chipSelect = BUILTIN_SDCARD; // Teensy 4.1 SD card pin


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); // this doesnt matter as much
  WxSerial.begin(4800); // DOES not work at any other baud rate
  Serial.println("Serial Communications established");
  setSyncProvider(getTeensy3Time);
  delay(100);
  if (timeStatus()!= timeSet) {
    Serial.println("Unable to sync with the RTC");
  } else {
    Serial.println("RTC has set the system time");
  }

    // SD card initialization
      // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
  }
  else {
    Serial.println("card initialized.");
    File sdCard = SD.open(fileName, FILE_WRITE);
    sdCard.println("--New data logging session started--");
    sdCard.close();

  }

}


void loop() {
  recvWithStartEndMarkers(); // read incoming serial data byte by byte using NMEA parsers
  saveAndShowNewData();
  
}


void recvWithStartEndMarkers() {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '$'; // NMEA start marker (not included)
    char endMarker = '*'; // sentence end markers
    char rc; // define incoming character

    while (WxSerial.available() > 0 && newData == false) {
        rc = WxSerial.read();

        if (recvInProgress == true) {
            if (rc != endMarker) {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= numChars) {
                    ndx = numChars - 1;
                }
            }
            else {
                receivedChars[ndx] = '\0'; // terminate the string
                recvInProgress = false;
                ndx = 0;
                newData = true;
            }
        }

        else if (rc == startMarker) {
            recvInProgress = true;
        }
    }
}

void saveAndShowNewData() {
    char sentenceHeader[5];
    // int i;
    if (newData == true) {
        // Serial.print("This just in ... ");
        Serial.println(receivedChars);
        // if (Serial.available() > 0) {
        //     for (i=0; i <5; i ++)
        //     Serial.print(receivedChars[i]);
        //     sentenceHeader[i] = receivedChars[i];
        // }
        

        //open and store the data
        File sdCard = SD.open(fileName, FILE_WRITE);
        sdCard.println(receivedChars);
        // if (sentenceHeader == 'GPGGA') {
        //     timeStampPacket(sdCard);
        // }


        sdCard.close();

        // reset the flag
        newData = false;
    }
}

void timeStampPacket(File df){
    // df = SD card file instance
    // DateTime now = timeStatus.now()
    sprintf(timeStamp, "Teensy Time Stamp:%04d-%02d-%02d %02d:%02d:%02d", year(), month(), day(), hour(), minute(), second());
    df.println(timeStamp);

}

time_t getTeensy3Time()
{
  return Teensy3Clock.get();
}