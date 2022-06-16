#include <SoftwareSerial.h>
SoftwareSerial mySerial(11, 10); // TX-Pin10, RX-Pin11
void updateSerial()
{
 delay(2000);
 while (Serial.available()) {
 mySerial.write(Serial.read());//Data received by Serial will be outputtedby mySerial
 }
 while(mySerial.available()) {
 Serial.write(mySerial.read());//Data received by mySerial will be outputted by Serial
 }
}
void setup()
{
 Serial.begin(19200);
 mySerial.begin(19200);
}
void loop()
{
 mySerial.println("AT"); // Once the handshake test is successful,it will back to OK
 updateSerial();
 mySerial.println("AT+CIPCLOSE"); //Disconnect the former connection
 updateSerial();
 mySerial.println("AT+CGATT=1 "); //The basic adhere network command of Internet connection
updateSerial();
 mySerial.println("AT+CGDCONT=1,\"IP\",\"CMNET\"");//Set PDP parameter
 updateSerial();
 mySerial.println("AT+CGACT=1,1");//Activate PDP; Internet connection is available after successful PDP activation
 updateSerial();
 mySerial.println("AT+CIFSR");//Get local IP address
 updateSerial();
 mySerial.println("AT+CIPSTART=TCP,118.26.119.118,8266");// Connect to the server then the server will send back former data
 updateSerial();
 updateSerial();
 delay(2000);
 updateSerial();
 mySerial.println("AT+CIPSEND");// Send data request to the server
 updateSerial();
 mySerial.print("TEST");// Send data to the server
 updateSerial();
 mySerial.write(26);// Terminator
 while(1)
 {
 if(mySerial.available())
 {
 Serial.write(mySerial.read());//Data received by mySerial will be outputted by Serial
 }
 if(Serial.available())
 {
 mySerial.write(Serial.read());//Data received by Serial will be outputted by mySerial
 }
 }
} 
