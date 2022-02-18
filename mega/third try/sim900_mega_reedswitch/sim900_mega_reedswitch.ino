#include <SoftwareSerial.h> //software serial library for serial communication b/w arduino & mySerial

SoftwareSerial mySerial(12, 13);//connect Tx pin of mySerial to pin 8 of arduino && Rx pin of mySerial to pin no 9 of arduino

char url[130];
char data[100];

void setup() {

  //GSM initialization
  mySerial.begin(19200);

  //Serial initialization
  Serial.begin(19200);

  //GSM config...
  Serial.println("Config SIM900...");
  delay(2000);
  Serial.println("Done!...");
  mySerial.flush();
  Serial.flush();

  // attach or detach from GPRS service 
  mySerial.println("AT+CGATT?");
  delay(100);
  toSerial();

  // bearer settings
  mySerial.println("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");
  delay(2000);
  toSerial();

  // bearer settings
  mySerial.println("AT+SAPBR=3,1,\"APN\",\"movistar.es\"");
  delay(2000);
  toSerial();
  delay(2000);

  // bearer settings
  mySerial.println("AT+SAPBR=1,1");
  delay(2000);
  toSerial();

  // bearer settings
  mySerial.println("AT+SAPBR=2,1");
  delay(2000);
  toSerial();
  //GSM config...

}

void loop() {
  // put your main code here, to run repeatedly:
  sendData();
  mySerial.println("");
  delay(10000);
}

void sendData() {
   // initialize http service
   mySerial.println("AT+HTTPINIT");
   delay(2000); 
   toSerial();
 
   // set http param value
   memset(data, 0, 100);
   memset(url, 0, 130);

   sprintf(data,"lema");
   sprintf(url,"AT+HTTPPARA=\"URL\",\"http://www.nrwlpms.com/sim900/get_data.php?pre=%s\"",data);
   mySerial.println(url);
   //mySerial.println("AT+HTTPPARA=\"URL\",\"http://www.nrwlpms.com/sim900/get_data.php?pre=5\"");

   delay(5000);
   toSerial();

   // set http action type 0 = GET, 1 = POST, 2 = HEAD
   mySerial.println("AT+HTTPACTION=0");
   delay(6000);
   toSerial();

   // read server response
   mySerial.println("AT+HTTPREAD"); 
   delay(1000);
   toSerial();
   delay(2000);

   mySerial.println("");
   mySerial.println("AT+HTTPTERM");
   toSerial();
   delay(300);

}

void toSerial()
{
  while(mySerial.available()!=0)
  {
    Serial.write(mySerial.read());
  }
}
