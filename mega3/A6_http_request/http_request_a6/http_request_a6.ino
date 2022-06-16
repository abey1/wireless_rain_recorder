
#include <SoftwareSerial.h>

#define OK 1
#define NOTOK 2
#define TIMEOUT 3
//#define RST 2

#define SERIALTIMEOUT 3000

SoftwareSerial A6board (12,13);



void setup() {
  // put your setup code here, to run once:
  A6board.begin(19200);   // the GPRS baud rate
  Serial.begin(19200);    // the GPRS baud rate

  if (A6begin() != OK) {
    Serial.println("Error");
    while (1 == 1);
  }

}

void loop() {
  // put your main code here, to run repeatedly:
  int s = 1;

  if(s == 1){
    SubmitHttpRequest();
    s = 2;
  }
}


void SubmitHttpRequest()
{
  A6command("AT+CSQ", "OK", "yy", 20000, 2);
  A6command("AT+CGATT?", "OK", "yy", 20000, 2);
  A6command("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"", "OK", "yy", 20000, 2); //setting the SAPBR, the connection type is using gprs
  A6command("AT+CIPSTATUS", "OK", "yy", 10000, 2);
  A6command("AT+SAPBR=3,1,\"gprs.swisscom.ch\",\"\"", "OK", "yy", 20000, 2); //setting the APN, the second need you fill in your local apn server
  A6command("AT+CIPSTATUS", "OK", "yy", 10000, 2);
  A6command("AT+SAPBR=1,1", "OK", "yy", 20000, 2); //setting the SAPBR, for detail you can refer to the AT command mamual
  A6board.println("AT+HTTPINIT"); //init the HTTP request

  delay(2000);
  ShowSerialData();

  //https://www.nrwlpms.com/sim900/a6_try_one.php?data=3
  //A6board.println("AT+HTTPPARA=\"URL\",\"www.google.com.hk\"");// setting the httppara, the second parameter is the website you want to access
  A6board.println("AT+HTTPPARA=\"URL\",\"www.nrwlpms.com/sim900/a6_try_one.php?data=3\"");
  delay(1000);

  ShowSerialData();

  A6board.println("AT+HTTPACTION=0");//submit the request
  delay(10000);//the delay is very important, the delay time is base on the return from the website, if the return datas are very large, the time required longer.
  //while(!mySerial.available());

  ShowSerialData();

  A6board.println("AT+HTTPREAD");// read the data from the website you access
  delay(300);

  ShowSerialData();

  A6board.println("");
  delay(100);
}

byte A6command(String command, String response1, String response2, int timeOut, int repetitions) {
  byte returnValue = NOTOK;
  byte count = 0;
  while (count < repetitions && returnValue != OK) {
    A6board.println(command);
    Serial.print("Command: ");
    Serial.println(command);
    if (A6waitFor(response1, response2, timeOut) == OK) {
      //     Serial.println("OK");
      returnValue = OK;
    } else returnValue = NOTOK;
    count++;
  }
  return returnValue;
}

void ShowSerialData()
{
  unsigned long entry = millis();
  while ( A6board.available() != 0 && millis() - entry < SERIALTIMEOUT)
    Serial.println(A6board.readStringUntil('\n'));
}

byte A6waitFor(String response1, String response2, int timeOut) {
  unsigned long entry = millis();
  int count = 0;
  String reply = A6read();
  byte retVal = 99;
  do {
    reply = A6read();
    if (reply != "") {
      Serial.print((millis() - entry));
      Serial.print(" ms ");
      Serial.println(reply);
    }
  } while ((reply.indexOf(response1) + reply.indexOf(response2) == -2) && millis() - entry < timeOut );
  if ((millis() - entry) >= timeOut) {
    retVal = TIMEOUT;
  } else {
    if (reply.indexOf(response1) + reply.indexOf(response2) > -2) retVal = OK;
    else retVal = NOTOK;
  }
  //  Serial.print("retVal = ");
  //  Serial.println(retVal);
  return retVal;
}

String A6read() {
  String reply = "";
  if (A6board.available())  {
    reply = A6board.readString();
  }
  //  Serial.print("Reply: ");
  //  Serial.println(reply);
  return reply;
}

bool A6begin() {
  A6board.println("AT+CREG?");
  byte hi = A6waitFor("1,", "5,", 1500);  // 1: registered, home network ; 5: registered, roaming
  while ( hi != OK) {
    A6board.println("AT+CREG?");
    hi = A6waitFor("1,", "5,", 1500);
  }

  if (A6command("AT&F0", "OK", "yy", 5000, 2) == OK) {   // Reset to factory settings
    if (A6command("ATE0", "OK", "yy", 5000, 2) == OK) {  // disable Echo
      if (A6command("AT+CMEE=2", "OK", "yy", 5000, 2) == OK) return OK;  // enable better error messages
      else return NOTOK;
    }
  }
}
