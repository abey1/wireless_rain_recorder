/*  SDlistFiles

 This example shows how print out the files in a directory on a SD card

 The circuit:
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4 Adafruit #2971 and Metro/Uno

 created   Nov 2010 by David A. Mellis
 modified 9 Apr 2012 by Tom Igoe
 modified 2 Feb 2014 by Scott Fitzgerald
 modified 12 Apr 2018 by Anne Barela

 This example code is in the public domain.
 
 */
#include <SPI.h>
#include <SD.h>

File root;
File myFile;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(19200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  pinMode(10, OUTPUT);      // set the SS pin as an output (necessary!)
  digitalWrite(10, HIGH);   // but turn off the W5100 chip
  
  Serial.print("Initializing SD card...");

  if (!SD.begin(53)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  root = SD.open("/");

  printDirectory(root, 0);

  Serial.println("done!");
}

void loop() {
  // nothing happens after setup finishes.
}

void printDirectory(File dir, int numTabs) {
  while (true) {

    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
      
      // reading the content of the file
      myFile = SD.open(entry.name());
      if (myFile) {
        // read from the file until there's nothing else in it:
        while (myFile.available()) {
          Serial.write(myFile.read());
        }
        // close the file:
        myFile.close();
      }else{
        // if the file didn't open, print an error:
        Serial.println("error opening test.txt");
      }

      //deleting file or removing file
      //if(entry){
        //SD.remove(entry.name());
        //Serial.println(strcat(entry.name(), " deleted"));
      //}
    }
    entry.close();
  }
}
