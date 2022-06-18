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

void setup() {

  // Initialize I2C communications as Master
  Wire.begin();
  
  // Setup serial monitor
  Serial.begin(9600);
  Serial.println("I2C Master Demonstration");
}

void loop() {
  delay(50);
  Serial.println("Write data to slave");
  
  sendDataToSlave("data_from_master");
    
  Serial.println("Receive data");
  
  // Read response from Slave
  // Read back 5 characters
  
  String response = recieveDataFromSlave();
  // Print to Serial Monitor
  Serial.println(response);
  
  while(1);
}

int sendDataToSlave(String data){

  int dataSize = sizeof(data)/sizeof(data[0]);

  // Setup byte variable in the correct size
  char information[dataSize];
  
  // Format answer as array
  for (byte i=0;i<dataSize;i++) {
    information[i] = (char)data.charAt(i);
  }
  
  // Write data to the Slave
  Wire.beginTransmission(SLAVE_ADDR);
  Wire.write(information);
  //Wire.write(data);
  Wire.endTransmission();
}

String recieveDataFromSlave(){
   Wire.requestFrom(SLAVE_ADDR,ANSWERSIZE);
  
  // Add characters to string
  String response = "";
  while (Wire.available()) {
      char b = Wire.read();
      response += b;
  }
  return response;
}
