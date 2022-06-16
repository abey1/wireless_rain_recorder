#include <SPI.h>
#include <SD.h>
File myFile;
void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);

  //wait until Serial port is open
  //  while (!Serial) {
  //    ; // wait for serial port to connect. Needed for native USB port only
  //  }

  initializeSDCard();
  
  writeToSdCard("data");
  
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

int writeToSdCard(char* data){
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open("test.txt", FILE_WRITE);
  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to test.txt...");
    //write data to file
    myFile.println(data);
    // close the file:
    myFile.close();
    Serial.println("done.");
   } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
}
