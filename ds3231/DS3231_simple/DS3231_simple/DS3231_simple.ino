/*
  DS3231: Real-Time Clock. Simple example
  Read more: www.jarzebski.pl/arduino/komponenty/zegar-czasu-rzeczywistego-rtc-ds3231.html
  GIT: https://github.com/jarzebski/Arduino-DS3231
  Web: http://www.jarzebski.pl
  (c) 2014 by Korneliusz Jarzebski
*/

#include <LiquidCrystal_I2C.h>

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 20, 4);

#include <Wire.h>
#include <DS3231.h>

DS3231 clock;
RTCDateTime dt;

long previousMillis = millis(); 
long interval = 10000;

//button for lighting backlight of lcd
const int buttonPin = 2;

// variables will change:
int buttonState = 0;         // variable for reading the pushbutton status

void setup()
{

  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT);
  
//  Serial.begin(19200);

  // Initialize DS3231
//  Serial.println("Initialize DS3231");;
  clock.begin();

  // initialize the LCD
  lcd.begin();

  // Turn on the blacklight and print a message.
  lcd.backlight();
  
  // Set sketch compiling time
  //clock.setDateTime(__DATE__, __TIME__);

}

void loop()
{
   // read the state of the pushbutton value:
  buttonState = digitalRead(buttonPin);
  
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

   lcd.setCursor(0,0);
  lcd.print(buf);
 
  //dims light of lcd after interval seconds
  if(millis() - previousMillis > interval) {
    lcd.noBacklight(); 
  }

  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (buttonState == LOW) {

    // set previousmillis to millis()
    previousMillis = millis();
    
    // turn lcd on:
    lcd.backlight();
  }
  
  delay(1000);
}
