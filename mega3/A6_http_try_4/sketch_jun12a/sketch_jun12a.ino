#include <SoftwareSerial.h>

SoftwareSerial gsm(12,13); //RX, TX

int chk;
int humi= 0;
int temp= 0;
void ShowSerialData()
{
 while(gsm.available()!=0)
   Serial.write(gsm.read());
}

void setup() {
  Serial.begin(19200);
  gsm.begin(19200);
  falan();
  singlecon();
}

void loop() {
 
//  Serial.print("Humidity: ");
//  Serial.print(DHT.humidity, 1);
//  Serial.print("%");
//  Serial.print(",\t");
//  Serial.print("Temperature: ");
//  Serial.print(DHT.temperature, 1);
//  Serial.println("*C");
//  temp = DHT.temperature;
//  humi = DHT.humidity;
  delay(2000);
  ShowSerialData();
  falan();
  singlecon();
  ShowSerialData();
}
void falan()
{
  Serial.println("Wait...");
  gsm.println("AT");
  delay(2000);
  ShowSerialData();
  gsm.println("AT+CPIN?");
  delay(2000);
  ShowSerialData();
  gsm.println("AT+CREG?");
  delay(2000);
  ShowSerialData();
  gsm.println("AT+CIPSHUT");
  delay(3000);
  ShowSerialData();
  gsm.println("AT+CIPSTATUS");
  
  delay(2000);
  ShowSerialData();
}
void singlecon()
{
  Serial.println("Wait2...");
  gsm.println("AT+CIPMUX=0");
  delay(2000);
  ShowSerialData();
  gsm.print("AT+CSTT=");
  gsm.print('"');
  gsm.print("internet");
  gsm.println('"');
  
  delay(2000);
  ShowSerialData();
  gsm.println("AT+CIICR");
  delay(2000);
  
  ShowSerialData();
  gsm.println("AT+CIFSR");
  delay(2000);
  
  ShowSerialData();
  gsm.print("AT+CIPSTART=");
  gsm.print('"');
  gsm.print("TCP");
  gsm.print('"');
  gsm.print(',');
  gsm.print('"');
  gsm.print("www.nrwlpms.com/sim900/a6_try_one.php?data=456");
  gsm.print('"');
  gsm.print(',');
  gsm.print("80");
  gsm.write(0x0d);
  gsm.write(0x0a);
  delay(2000);
  ShowSerialData();
  gsm.println("AT+CIPSEND");
  delay(5000);
  ShowSerialData();
  gsm.print("GET");
  gsm.print(' ');
  //gsm.print("http:");
  //gsm.print('/');
  //gsm.print('/');
  //gsm.print("api.thingspeak.com");
  gsm.print('/');
  gsm.print("/sim900/a6_try_one.php?data=");
  gsm.print("123");//my API Key
//  gsm.print("&field1=");
//  gsm.print(temp);
//  gsm.print("&field2=");
//  gsm.print(humi);
  
  gsm.write(0x0d);
  gsm.write(0x0a);
  
  gsm.write(0x1a); // the trick is here to send the request. Its Ctrl+Z to start send process.
  delay(15000);
  
  ShowSerialData();
  
  //gsm.println("AT+CIPSHUT");
  gsm.println("AT+CIPCLOSE");
  gsm.write(0x0d);
  gsm.write(0x0a);
  delay(30000);
  ShowSerialData(); 
}
