/*
  I2C Master Demo
  i2c-master-demo.ino
  Demonstrate use of I2C bus
  Master sends character and gets reply from Slave
  DroneBot Workshop 2019
  https://dronebotworkshop.com
*/

// Include Arduino Wire library for I2C
#include <Wire.h>

// Define Slave I2C Address
#define SLAVE_ADDR 9

// Define Slave answer size
#define ANSWERSIZE 5

#define START_SENDING 0

#define ACCEPTED "accep"
#define BUSY "busyy"

#include <SPI.h>
#include <SD.h>

File nextFile;

char longFile[9] = "long.TXT"; 
char latFile[8] = "lat.TXT"; 

char buffer[2000];

int noOfFile = 17;

byte pos = 0;

int upToNoOfFile = 1;

char sNoOfFile[10];

char* DATA = "55,217,2022-06-02_09:11:23|0,2022-06-02_09:11:17|0,2022-06-02_09:11:12|0,2022-06-02_09:04:25|0,2022-06-02_06:48:22|0,2022-06-02_06:48:16|0,2022-06-02_06:48:11|0,2022-06-02_06:48:05|0,2022-06-02_06:42:57|0,2022-06-02_06:42:51|0";
       
void resetBuffer() {
  memset(buffer, 0, sizeof(buffer));
  pos = 0;
}

void setup() {
  
  // Initialize I2C communications as Master
  Wire.begin();
  
  // Setup serial monitor
  Serial.begin(9600);
  Serial.println("I2C Master Demonstration");
}

void loop() {
  delay(50);

//  Serial.println(DATA);
//  int dataSize = strlen(DATA);
//  
//  resetBuffer();
//  
//  for(int i = 0; i < dataSize; i++){
//    buffer[pos++] = DATA[i];
//  }

  readFromSdCard();
  
  sendDataToSlave();

 
  delay(500);
  
  receiveDataFromSlave();
  
  //readFromSdCard();
  //delay(500);
  //writeToSdCard();
  //delay(500);
  //readFromSdCard();
  //writeDataToSlave(START_SENDING);
  //readFromSdCard();
  //receiveDataFromSlave();
  Serial.println("continue work");
  while(1);
  //writeDataToSlave();
    
  //receiveDataFromSlave();
}

void sendDataToSlave(){
  char temp[32];
  for(int i = 0; i <= pos; i+=32){  
    int k = 0;
    for(int j = i; j < i+32; j++){
      temp[k] = buffer[j];
      k++;
    }
    Serial.println(temp);
    Wire.beginTransmission(SLAVE_ADDR);
    Wire.write(temp);
    Wire.endTransmission();
    delay(500);
  }
}

int writeDataToSlave(byte order){
  Serial.println("Write data to slave");
  
  // Write a charatre to the Slave
  Wire.beginTransmission(SLAVE_ADDR);
  Wire.write("55,217,2022-06-02_09:11:23|0,2022-06-02_09:11:17|0,2022-06-02_09:11:12|0,2022-06-02_09:04:25|0,2022-06-02_06:48:22|0,2022-06-02_06:48:16|0,2022-06-02_06:48:11|0,2022-06-02_06:48:05|0,2022-06-02_06:42:57|0,2022-06-02_06:42:51|0");
  Wire.endTransmission();
}

int receiveDataFromSlave(){
  Serial.println("Receive data");
  
  // Read response from Slave
  // Read back 5 characters
  Wire.requestFrom(SLAVE_ADDR,ANSWERSIZE);
  
  // Add characters to string
  String response = "";
  while (Wire.available()) {
      char b = Wire.read();
      response += b;
  } 
  
  // Print to Serial Monitor
  Serial.println(response);
  if(response == ACCEPTED){
    deleteFiles();
  }

  if(response == BUSY){
    Serial.println("do nothing...");
  }
}

void deleteFiles(){
  Serial.println("file deletion...");
}

int readFromSdCard(){

  //initialize sd card
  if(!SD.begin(53)){
    Serial.println("initialization failed. Things to check:");
    Serial.println("1. is a card inserted?");
    Serial.println("2. is your wiring correct?");
    Serial.println("3. did you change the chipSelect pin to match your shield or module?");
    Serial.println("Note: press reset or reopen this Serial Monitor after fixing your issue!");
  }else{
    resetBuffer();
    
    //read long file
    nextFile = SD.open(longFile);
    if(nextFile){
      while (nextFile.available()) {
          //Serial.write(nextFile.read());
          buffer[pos++] = nextFile.read();
      }
      buffer[pos++] = ',';
      nextFile.close();
    }
    
    //read lat file
    nextFile = SD.open(latFile);
    if(nextFile){
      while (nextFile.available()) {
          //Serial.write(nextFile.read());
          buffer[pos++] = nextFile.read();
      }
      buffer[pos++] = ',';
      nextFile.close();
    }
  
    int range;
    
    if(noOfFile > 5)
      range = 5;
    else
      range = noOfFile;
      
    int limit = noOfFile - range;
    
    for(upToNoOfFile = noOfFile; upToNoOfFile > limit; upToNoOfFile--){
      
      itoa(upToNoOfFile,sNoOfFile,10);
      strcat(sNoOfFile, ".TXT");
      nextFile = SD.open(sNoOfFile);
      Serial.println(sNoOfFile);
      
      if (nextFile) {
        while (nextFile.available()) {
          buffer[pos++] = nextFile.read();
        }
  
        nextFile.close();
      } else {
        Serial.println("error opening file from read");
      }
    }
    Serial.write("--------------->");
    Serial.write(buffer);
  }
}

//int writeToSdCard(){
//
//  //initialize sd card
//  if(!SD.begin(13)){
//    Serial.println("initialization failed. Things to check:");
//    Serial.println("1. is a card inserted?");
//    Serial.println("2. is your wiring correct?");
//    Serial.println("3. did you change the chipSelect pin to match your shield or module?");
//    Serial.println("Note: press reset or reopen this Serial Monitor after fixing your issue!");
//  }else{
//    
//    Serial.println(sTempFileName);
//    nextFile = SD.open(sTempFileName, FILE_WRITE);
//    if(nextFile){
//      
//      nextFile.print(buffer);
//      Serial.println("done writing to second sd card");
//      nextFile.close();
//    }
//  }
//}
