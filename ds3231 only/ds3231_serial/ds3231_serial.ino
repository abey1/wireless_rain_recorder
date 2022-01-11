#include <Wire.h>
#include <DS3231.h>

DS3231 clock;
RTCDateTime dt;

void setup()
{
  Serial.begin(19200);
  
  // Initialize DS3231
  //  Serial.println("Initialize DS3231");;
  clock.begin();

  // Set sketch compiling time
  //clock.setDateTime(__DATE__, __TIME__);

}

void loop()
{
  dt = clock.getDateTime();

  // For leading zero look to DS3231_dateformat example

//  Serial.print("Raw data: ");
//  Serial.print(dt.year);   Serial.print("-");
//  Serial.print(dt.month);  Serial.print("-");
//  Serial.print(dt.day);    Serial.print(" ");
//  Serial.print(dt.hour);   Serial.print(":");
//  Serial.print(dt.minute); Serial.print(":");
//  Serial.print(dt.second); Serial.println("");


// Format the time and date and insert into the temporary buffer.
  char buf[50];
  snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
           dt.year, dt.month, dt.day,
           dt.hour, dt.minute, dt.second);


  Serial.println(buf);
}
