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

#include <DS3231.h>
#include <SPI.h>
#include <SD.h>
DS3231 clock;
RTCDateTime dt;
 
// Define Slave I2C Address
#define SLAVE_ADDR 9

// Define Slave answer size
#define ANSWERSIZE 5

#define IDLEE "idlee"
#define BUSYY "busyy"

File nextFile;

char buffer[32];

int noOfFile = 0;

byte pos = 0;

int upToNoOfFile = 1;

//variable to hold noOfFile changed to string
char sNoOfFile[10];

const int  buttonPin = 2;    // the pin that the reedswitch is attached to
int buttonPushCounter = 0; // counter for the number of times reed switch detects magnet presses
int buttonState = 0;         // current state of the reed switch

//this two variables holds and displays logitude and latitude
char longitude = "21";
char latitude = "7";


long previousMillisSd;
long sendingGap;

long intervalSd = 900000;
long sendingInterval = 30000;

//pin number on mega to access sd card
const int chipSelect = 53;

//timebuf to hold current time
char timeBuf[20];

//
int noRainCounter = 0;

// end of file
const char endOfFileChar = 'E';

void resetBuffer() {
  memset(buffer, 0, sizeof(buffer));
  pos = 0;
}

int getLastNoOfFile(){
  //this function will read and continue from old saved noOfFile if power is out
  int tempNoOfFile = 1;
  char sTempNoOfFile[10];
  //a stands for available
  bool a = false;
  do{
    itoa(tempNoOfFile,sTempNoOfFile,10);
    
    strcat(sTempNoOfFile, ".TXT");
    // re-open the file for reading:
    //sNoOfFile is used here because it is concatinated to .txt file before in write
    nextFile = SD.open(sTempNoOfFile);
    if(nextFile){
      a = true;
      noOfFile = tempNoOfFile;
      Serial.print("found");
      Serial.println(sTempNoOfFile);
      nextFile.close();
      tempNoOfFile++;
      delay(100);
    }else{
      a = false;
    }
  }while(a);
}

void setup() {
  
  // Initialize I2C communications as Master
  Wire.begin();
  
  // Setup serial monitor
  Serial.begin(9600);
  Serial.println("I2C Master Demonstration");

  //begin clock
  clock.begin();
  // uncomment this to assign compiling date and time
  // Set sketch compiling time
  //clock.setDateTime(__DATE__, __TIME__);

  //initialize sd card
  if(!SD.begin(chipSelect)){
    Serial.println("initialization failed. Things to check:");
    Serial.println("1. is a card inserted?");
    Serial.println("2. is your wiring correct?");
    Serial.println("3. did you change the chipSelect pin to match your shield or module?");
    Serial.println("Note: press reset or reopen this Serial Monitor after fixing your issue!");
    while (true);
  }

  //assign previousMillisSd current time
  sendingGap = millis();
  previousMillisSd = millis();
  getLastNoOfFile();
}

void loop() {
  
  //get date and time from clock
  dt = clock.getDateTime();

  //read from reed switch
  buttonState = digitalRead(buttonPin);

  //tipping bucket sd saver
  if (buttonState != HIGH) {
     buttonPushCounter++;
     //save to file
     writeToSdCard('1');
     delay(100);
     if(isSlaveIdle()){
        if(readFromSdCard()){
          sendDataToSlave();
        }
      }
     
     //reset default saver
     previousMillisSd = millis();
     //subtract one from no-rain counter
     if(noRainCounter > 0){
      noRainCounter--;
     }
     //add one to the number of files
     //noOfFile++;
     
     Serial.println("on");
     Serial.print("number of button pushes: ");
     Serial.println(buttonPushCounter);
   }

  //periodically saves value of 0 to sdreader if 
  //no rain appeared for some time
  if(millis() - previousMillisSd > intervalSd){
    if(noOfFile == 0){
       writeToSdCard('0');
       delay(100);
       if(isSlaveIdle()){
          if(readFromSdCard()){
            sendDataToSlave();
          }
       }   
    }
    previousMillisSd = millis();
  }
  

  //send every 5 minutes after once start sending
  if(millis() - sendingGap > sendingInterval) {
    if(noOfFile > 0){
      if(isSlaveIdle()){
        if(readFromSdCard()){
           sendDataToSlave();
        }
      }
    }
    //send every 5 minutes after once start sending
    sendingGap = millis();
  }
  
  snprintf(timeBuf, sizeof(timeBuf), "%04d-%02d-%02d_%02d:%02d:%02d",
           dt.year, dt.month, dt.day,
           dt.hour, dt.minute, dt.second);
}

int writeToSdCard(char amount){
  itoa(++noOfFile,sNoOfFile,10);
  strcat(sNoOfFile, ".TXT");
  SD.remove(sNoOfFile);
  nextFile = SD.open(sNoOfFile, FILE_WRITE);
  if(nextFile){
      Serial.println(timeBuf);
      nextFile.print(timeBuf);
      nextFile.print("|");
      nextFile.print(amount);
      nextFile.print(",");
      nextFile.close();
  }
}

void sendDataToSlave(){
  char temp[32];
  Serial.println("");
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

bool isSlaveIdle(){
  bool idle = true;
  
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
  if(response == BUSYY){
    idle = false;
  }

  return idle;
}

bool readFromSdCard(){

    bool readCorrect = true;
 
    resetBuffer();
  
    int range = 1;
      
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
        buffer[pos++] = endOfFileChar;
        Serial.write("--------------->");
        Serial.write(buffer);
        deleteSingleFile();
        return readCorrect;
      } else {
        Serial.println("error opening file for read");
        //deleteSingleFile();
        readCorrect = false;
        return readCorrect;
      }
    }
  
}

int deleteSingleFile(){
    itoa(noOfFile,sNoOfFile,10);
    strcat(sNoOfFile, ".TXT");
    Serial.print("deleting");
    Serial.println(sNoOfFile);
    SD.remove(sNoOfFile);
    noOfFile--;
}
