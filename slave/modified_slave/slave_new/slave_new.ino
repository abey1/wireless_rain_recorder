/*
  I2C Slave Demo
  i2c-slave-demo.ino
  Demonstrate use of I2C bus
  Slave receives character from Master and responds
  DroneBot Workshop 2019
  https://dronebotworkshop.com
*/



// Include Arduino Wire library for I2C
#include <Wire.h>

#include <SPI.h>
#include <SD.h>

// Define Slave I2C Address
#define SLAVE_ADDR 9

// Define Slave answer size
//#define ANSWERSIZE 5

#define START_SENDING 0

#include <SoftwareSerial.h>

#include "SIM800L.h"

#define SIM800_RX_PIN 10
#define SIM800_TX_PIN 11
#define SIM800_RST_PIN 6

const char APN[] = "Internet.be";

SIM800L* sim800l;

char buffer[2000];

byte pos = 0;

File nextFile;

// Define string with response to Master
//String answer = "Hello";

char sTempFileName[9] = "temp.TXT";

void setup() {

  // Initialize I2C communications as Slave
  Wire.begin(SLAVE_ADDR);
  
  // Function to run when data requested from master
  Wire.onRequest(requestEvent); 
  
  // Function to run when data received from master
  Wire.onReceive(receiveEvent);

  // Initialize a SoftwareSerial
  SoftwareSerial* serial = new SoftwareSerial(SIM800_RX_PIN, SIM800_TX_PIN);
  serial->begin(9600);
  delay(1000);
   
  // Initialize SIM800L driver with an internal buffer of 200 bytes and a reception buffer of 512 bytes, debug disabled
  //sim800l = new SIM800L((Stream *)serial, SIM800_RST_PIN, 200, 512);

  // Equivalent line with the debug enabled on the Serial
  sim800l = new SIM800L((Stream *)serial, SIM800_RST_PIN, 200, 512, (Stream *)&Serial);

  // Setup module for GPRS communication
  setupModule();

  if(!SD.begin(53)){
    Serial.println("initialization failed. Things to check:");
    Serial.println("1. is a card inserted?");
    Serial.println("2. is your wiring correct?");
    Serial.println("3. did you change the chipSelect pin to match your shield or module?");
    Serial.println("Note: press reset or reopen this Serial Monitor after fixing your issue!");
    while(1);
  }
  
  // Setup Serial Monitor 
  Serial.begin(9600);
  Serial.println("I2C Slave Demonstration");
  resetBuffer();
}

//void receiveEvent() {
//
//  // Read while data received
//  byte x;
//  while (0 < Wire.available()) {
//    x = Wire.read();
//  }
//
//  if(x == START_SENDING){
//    Serial.println("yes master i am sending");
//    delay(500);
//    readFromSdCard();
//  }else{
//    Serial.println("wrong order");
//  }
//  
//  // Print to Serial Monitor
//  
//}

void requestEvent() {

  //check if there is file in SD card
  if(fileExists()){
    sendAnswer("busyy", 5);
  }else{
    writeToSdCard();
    if(fileExists()){
      sendAnswer("accep", 5);
    }
  }
  Serial.println("Request event");
}

void sendAnswer(String answer, int ANSWERSIZE){
  byte response[ANSWERSIZE];
  
  // Format answer as array
  for (byte i=0;i<ANSWERSIZE;i++) {
    response[i] = (byte)answer.charAt(i);
  }
  
  // Send response back to Master
  Wire.write(response,sizeof(response));
}

void startSending(){
  Serial.println("sending...");
  sendDataToInternet0();
}

void resetBuffer() {
  memset(buffer, 0, sizeof(buffer));
  pos = 0;
}

int readFromSdCard(){

  //initialize sd card
  
  resetBuffer();
  nextFile = SD.open("temp.txt");
    
  if (nextFile) {
    while (nextFile.available()) {
      buffer[pos++] = nextFile.read();
    }
   nextFile.close();
  }else {
    Serial.println("error opening file from read");
  }
  Serial.write("-----slave--->");
  Serial.write(buffer);
}

void loop() {

  if(fileExists()){
    readFromSdCard();
    startSending();
  }
 
  // Time delay in loop
  delay(1000);
}

void receiveEvent() {
  
  while (0 < Wire.available()) { // loop through all but the last
    char c = Wire.read(); // receive byte as a character
    buffer[pos++] = c;
    //Serial.print(c);         // print the character
  }
  
  //int x = Wire.read();    // receive byte as an integer
  //Serial.println(x);         // print the integer
}

//void receiveEvent(int howMany) {
//  while (0 < Wire.available()) { // loop through all but the last
//    char c = Wire.read(); // receive byte as a character
//    Serial.print(c);         // print the character
//  }
//  //int x = Wire.read();    // receive byte as an integer
//  //Serial.println(x);         // print the integer
//}


bool fileExists(){
  bool exist = false;
  
 
    nextFile = SD.open(sTempFileName, FILE_READ);
    if(nextFile){
      exist = true;
    }else{
      Serial.print("cant read sd card");
    }
  
  return exist;
}

int writeToSdCard(){

  //initialize sd card
  if(!SD.begin(53)){
    Serial.println("initialization failed. Things to check:");
    Serial.println("1. is a card inserted?");
    Serial.println("2. is your wiring correct?");
    Serial.println("3. did you change the chipSelect pin to match your shield or module?");
    Serial.println("Note: press reset or reopen this Serial Monitor after fixing your issue!");
  }else{
    Serial.println(sTempFileName);
    nextFile = SD.open(sTempFileName, FILE_WRITE);
    if(nextFile){
      
      nextFile.print(buffer);
      Serial.println("done writing to second sd card");
      nextFile.close();
      resetBuffer();
    }
  }
}

char* concatinate(char s1[], char s2[]){
  //char s1[100] = "programming ", s2[] = "is awesome";
  int length, j;

  // store length of s1 in the length variable
  length = 0;
  while (s1[length] != '\0') {
    ++length;
  }

  // concatenate s2 to s1
  for (j = 0; s2[j] != '\0'; ++j, ++length) {
    s1[length] = s2[j];
  }

  // terminating the s1 string
  s1[length] = '\0';

  return s1;
}

void sendDataToInternet0(){


//  char* U = "http://www.nrwlpms.com/sim900/a6_try_one.php?data=";
//  //strcat(U,"o_d");
//
//  U = concatinate(U, "o_d");
//  
//  sendDataToInternet(U);

  
  char* U = "http://www.nrwlpms.com/sim900/save_data4.php?data=";
  //strcat(U,"o_d");

  U = concatinate(U, buffer);
  
  sendDataToInternet(U);
}

int sendDataToInternet(char* URL){
//  int l = sizeof(url)/sizeof(url[0]);
//  
//  char URL[l] = url;
  
  Serial.println(URL);
  // Establish GPRS connectivity (5 trials)
  bool connected = false;
  
  for(uint8_t i = 0; i < 5 && !connected; i++) {
    delay(1000);
    connected = sim800l->connectGPRS();
  }

  // Check if connected, if not reset the module and setup the config again
  if(connected) {
    Serial.print(F("GPRS connected with IP "));
    Serial.println(sim800l->getIP());
  } else {
    Serial.println(F("GPRS not connected !"));
    Serial.println(F("Reset the module."));
    sim800l->reset();
    setupModule();
    return;
  }

  Serial.println(F("Start HTTP GET..."));

  // Do HTTP GET communication with 10s for the timeout (read)
  uint16_t rc = sim800l->doGet(URL, 10000);
   if(rc == 200) {
    // Success, output the data received on the serial
    Serial.print(F("HTTP GET successful ("));
    Serial.print(sim800l->getDataSizeReceived());
    Serial.println(F(" bytes)"));
    Serial.print(F("Received : "));
    String s = sim800l->getDataReceived();
    Serial.print(s);
    if(s == "^"){
      deleteFile();
      Serial.print("file deleted");
    }else{
      Serial.print("error");
    }
  } else {
    // Failed...
    Serial.print(F("HTTP GET error "));
    Serial.println(rc);
  }

  delay(1000);

  // Close GPRS connectivity (5 trials)
  bool disconnected = sim800l->disconnectGPRS();
  for(uint8_t i = 0; i < 5 && !connected; i++) {
    delay(1000);
    disconnected = sim800l->disconnectGPRS();
  }
  
  if(disconnected) {
    Serial.println(F("GPRS disconnected !"));
  } else {
    Serial.println(F("GPRS still connected !"));
  }

  // Go into low power mode
  bool lowPowerMode = sim800l->setPowerMode(MINIMUM);
  if(lowPowerMode) {
    Serial.println(F("Module in low power mode"));
  } else {
    Serial.println(F("Failed to switch module to low power mode"));
  }
}

void setupModule() {
    // Wait until the module is ready to accept AT commands
  while(!sim800l->isReady()) {
    Serial.println(F("Problem to initialize AT command, retry in 1 sec"));
    delay(1000);
  }
  Serial.println(F("Setup Complete!"));

  // Wait for the GSM signal
  uint8_t signal = sim800l->getSignal();
  while(signal <= 0) {
    delay(1000);
    signal = sim800l->getSignal();
  }
  Serial.print(F("Signal OK (strenght: "));
  Serial.print(signal);
  Serial.println(F(")"));
  delay(1000);

  // Wait for operator network registration (national or roaming network)
  NetworkRegistration network = sim800l->getRegistrationStatus();
  while(network != REGISTERED_HOME && network != REGISTERED_ROAMING) {
    delay(1000);
    network = sim800l->getRegistrationStatus();
  }
  Serial.println(F("Network registration OK"));
  delay(1000);

  // Setup APN for GPRS configuration
  bool success = sim800l->setupGPRS(APN);
  while(!success) {
    success = sim800l->setupGPRS(APN);
    delay(5000);
  }
  Serial.println(F("GPRS config OK"));
}

int deleteFile(){  
  //initialize sd card
  if(!SD.begin(53)){
    Serial.println("initialization failed. Things to check:");
    Serial.println("1. is a card inserted?");
    Serial.println("2. is your wiring correct?");
    Serial.println("3. did you change the chipSelect pin to match your shield or module?");
    Serial.println("Note: press reset or reopen this Serial Monitor after fixing your issue!");
  }else{
    SD.remove("temp.txt");
  }
  
}
