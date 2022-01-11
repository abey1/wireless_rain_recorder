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

  // Format the time and date and insert into the temporary buffer.
  char buf[50];
  snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
           dt.year, dt.month, dt.day,
           dt.hour, dt.minute, dt.second);


  Serial.println(buf);
}
