#include <SPI.h>
#include <SD.h>
File myFile;
void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  
  //while (!Serial) {
  //  ; // wait for serial port to connect. Needed for native USB port only
  //}
  
  initializeSDCard();

  readSDCard();
}

void loop() {
// nothing happens after setup
}

int initializeSDCard(){
    Serial.print("Initializing SD card...");
  if (!SD.begin(53)) {
    Serial.println("initialization failed!");
    while (1);
  }
  
  Serial.println("initialization done.");
}

int readSDCard(){
    // open the file for reading:
  myFile = SD.open("test.txt");
  if (myFile) {
  Serial.println("test.txt:");
  
  // read from the file until there's nothing else in it:
  while (myFile.available()) {
    Serial.write(myFile.read());
  }
  
  // close the file:
  myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
}
