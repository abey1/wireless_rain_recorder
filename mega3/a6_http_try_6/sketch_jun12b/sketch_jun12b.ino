#include <SoftwareSerial.h>
SoftwareSerial myGsm(10,11);
void updateSerial()
{
  delay(2000);
  while (Serial.available()) {
    myGsm.write(Serial.read());//Data received by Serial will be outputted by mySerial
  }
  while (myGsm.available()) {
    Serial.write(myGsm.read());//Data received by myGsm will be outputted by Serial
}
  
}
void setup() {
  myGsm.begin(19200);
  Serial.begin(19200);
  delay(500);
  myGsm.println("AT+CIPSHUT\r"); //RESPONSE= OK
  updateSerial();
  myGsm.println("AT+CIPMUX=0\r"); //RESPONSE= OK
  updateSerial();
  myGsm.println("AT+CGATT=1\r"); //RESPONSE= OK
 updateSerial();
  myGsm.println("AT+CSTT=\"internet\",\"\",\"\"\r"); //RESPONSE= OK
  updateSerial();
  myGsm.println("AT+CIICR\r"); //RESPONSE= OK
  updateSerial();
  myGsm.println("AT+CIFSR\r"); //RESPONSE= Returns an IP
 updateSerial();
  myGsm.println("AT+CIPSTART=\"TCP\",\"www.nrwlpms.com/sim900/a6_try_one.php?data=22\", 80\r"); //RESPONSE= CONNECTED OK
  updateSerial();
  myGsm.println("AT+CIPSEND\r"); //RESPONSE= >
  updateSerial();
  myGsm.println("GET www.nrwlpms.com/sim900/a6_try_one.php?data=990/ HTTP/1.1");
  updateSerial();
  myGsm.println("Host: nrwlpms.com");
  updateSerial();
  myGsm.println("Content-Type: application/json");
 updateSerial();
  myGsm.println("Content-Length: 25\r\n");
  updateSerial();
  myGsm.println("{\"Celsius\":\"TEMPERATURE\"}");
  updateSerial();
  myGsm.write(0x1A); // Ctrl Z
  delay(3000);
  updateSerial();
  /*
    After sending all these instructions, I get the following response,
    OK
    HTTP/1.1 200 OK
    Friday December, 22
    +TCPCLOSE=0
    OK
  */
  myGsm.println("AT+CIPCLOSE"); //RESPONSE= OK
  updateSerial();
  myGsm.println("AT+CIPSHUT"); //RESPONSE= OK
 updateSerial();
}
void loop() {
}
