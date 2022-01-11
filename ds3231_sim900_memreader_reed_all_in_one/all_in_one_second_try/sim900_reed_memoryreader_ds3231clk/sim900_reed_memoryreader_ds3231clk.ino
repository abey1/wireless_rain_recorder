#include <SoftwareSerial.h>
SoftwareSerial GSM(7, 8); // RX, TX

//clock imports start
#include <Wire.h>
#include <DS3231.h>

DS3231 clock;
RTCDateTime dt;
//clock imports end

//sd card imports begin
#include <SD.h>

const int chipSelect = 10;
File myFile;
//sd card imports end

// this constant won't change:
const int  buttonPin = 2;    // the pin that the pushbutton is attached to
const int ledPin = 13;       // the pin that the LED is attached to

// Variables will change:
int buttonPushCounter = 0;   // counter for the number of button presses
int buttonState = 0;         // current state of the button
int lastButtonState = 0;     // previous state of the button

enum _parseState {
  PS_DETECT_MSG_TYPE,

  PS_IGNORING_COMMAND_ECHO,

  PS_HTTPACTION_TYPE,
  PS_HTTPACTION_RESULT,
  PS_HTTPACTION_LENGTH,

  PS_HTTPREAD_LENGTH,
  PS_HTTPREAD_CONTENT
};

enum _actionState{
  AS_IDLE,
  AS_WAITING_FOR_RESPONSE  
};

unsigned long data = millis();
char url[130];
byte actionState = AS_IDLE;
unsigned long lastActionTime = 0;

byte parseState = PS_DETECT_MSG_TYPE;
char buffer[80];
byte pos = 0;

int contentLength = 0;

void resetBuffer() {
  memset(buffer, 0, sizeof(buffer));
  pos = 0;
}

void sendGSM(const char* msg, int waitMs = 500) {
  GSM.println(msg);
  delay(waitMs);
  while(GSM.available()) {
    parseATText(GSM.read());
  }
}

void setup()
{

  // Initialize DS3231
  clock.begin();
  // initialize the button pin as a input:
  pinMode(buttonPin, INPUT);
  // initialize the LED as an output:
  pinMode(ledPin, OUTPUT);
  SD.begin(chipSelect);

//  Serial.print("Initializing SD card...");
//
//  if (!SD.begin(chipSelect)) {
//    Serial.println("initialization failed. Things to check:");
//    Serial.println("1. is a card inserted?");
//    Serial.println("2. is your wiring correct?");
//    Serial.println("3. did you change the chipSelect pin to match your shield or module?");
//    Serial.println("Note: press reset or reopen this Serial Monitor after fixing your issue!");
//    while (true);
//  }

  // initialize serial communication:
  //initialize gsm
  GSM.begin(19200);
  Serial.begin(19200);
  
  sendGSM("AT+SAPBR=3,1,\"APN\",\"internet\"");  
  sendGSM("AT+SAPBR=1,1",3000);
  sendGSM("AT+HTTPINIT");  
  sendGSM("AT+HTTPPARA=\"CID\",1");
  //sprintf(url,"AT+HTTPPARA=\"URL\",\"http://www.halluuethio.com/IOT/receive_number_loop.php?number=%d\"",buttonPushCounter);
  //sendGSM(url);
  //sendGSM("AT+HTTPPARA=\"URL\",\"http://www.halluuethio.com/IOT/receive_number_loop.php?number=33\"");
}

void loop()
{ 
  //REEDSWITCH counter
  // read the pushbutton input pin:
  buttonState = digitalRead(buttonPin);

  // compare the buttonState to its previous state
  if (buttonState != lastButtonState) {
    // if the state has changed, increment the counter
    if (buttonState == HIGH) {
      // if the current state is HIGH then the button went from off to on:
      //Serial.println("off");
    } else {
      // if the current state is LOW then the button went from on to off:
      
      buttonPushCounter++;
      Serial.println("on");
      Serial.print("number of button pushes: ");
      Serial.println(buttonPushCounter);
    }
    // Delay a little bit to avoid bouncing
    delay(50);
  }
  // save the current state as the last state, for next time through the loop
  lastButtonState = buttonState;


  // turns on the LED every four button pushes by checking the modulo of the
  // button push counter. the modulo function gives you the remainder of the
  // division of two numbers:
  if (buttonPushCounter % 4 == 0) {
    digitalWrite(ledPin, HIGH);
  } else {
    digitalWrite(ledPin, LOW);
  }
  //REEDSWITCH counter
  
  //GSM data sender
  data = millis();
  
  unsigned long now = millis();

  if(actionState == AS_IDLE){
    if(now > lastActionTime + 10000){

      

      //get time and counted to be written
      dt = clock.getDateTime();
      
      // Format the time and date and insert into the temporary buffer.
      char buf[30];
      snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d<->%02d",
               dt.year, dt.month, dt.day,
               dt.hour, dt.minute, dt.second,buttonPushCounter);
      Serial.println(buf);

      //write to memory start

      myFile = SD.open("test.txt", FILE_WRITE);

      // if the file opened okay, write to it:
      if (myFile) {
        Serial.print("Writing to test.txt...");
        myFile.println(buf);
        // close the file:
        myFile.close();
        Serial.println("done.");
      } else {
        // if the file didn't open, print an error:
        Serial.println("error opening test.txt");
      }
      //write to memory end

      //read from memory start
      // re-open the file for reading:
      myFile = SD.open("test.txt");
      if (myFile) {
        Serial.println("test.txt:");
    
        // read from the file until there's nothing else in it:
        while (myFile.available()) {
          Serial.write(myFile.read());
        }
        // close the file:
        myFile.close();
      } else {
        // if the file didn't open, print an error:
        Serial.println("error opening test.txt");
      }
      //read from memory end
      //sprintf(url,"AT+HTTPPARA=\"URL\",\"http://www.halluuethio.com/IOT/receive_number_loop.php?number=%d\"",buttonPushCounter);
      sprintf(url,"AT+HTTPPARA=\"URL\",\"http://www.nrwlpms.com/sim900/get_data.php?pre=%d\"",buttonPushCounter);
      sendGSM(url);
      sendGSM("AT+HTTPACTION=0");
      buttonPushCounter=0;
      lastActionTime = now;
      actionState = AS_WAITING_FOR_RESPONSE;
    }
  }
   
  while(GSM.available()) {
    lastActionTime = now;
    parseATText(GSM.read());
  }
  //GSM data sender
}

void parseATText(byte b) {

  buffer[pos++] = b;

  if ( pos >= sizeof(buffer) )
    resetBuffer(); // just to be safe

  switch (parseState) {
  case PS_DETECT_MSG_TYPE: 
    {
      if ( b == '\n' )
        resetBuffer();
      else {        
        if ( pos == 3 && strcmp(buffer, "AT+") == 0 ) {
          parseState = PS_IGNORING_COMMAND_ECHO;
        }
        else if ( b == ':' ) {
          //Serial.print("Checking message type: ");
          //Serial.println(buffer);

          if ( strcmp(buffer, "+HTTPACTION:") == 0 ) {
            Serial.println("Received HTTPACTION");
            parseState = PS_HTTPACTION_TYPE;
          }
          else if ( strcmp(buffer, "+HTTPREAD:") == 0 ) {
            Serial.println("Received HTTPREAD");            
            parseState = PS_HTTPREAD_LENGTH;
          }
          resetBuffer();
        }
      }
    }
    break;

  case PS_IGNORING_COMMAND_ECHO:
    {
      if ( b == '\n' ) {
        Serial.print("Ignoring echo: ");
        Serial.println(buffer);
        parseState = PS_DETECT_MSG_TYPE;
        resetBuffer();
      }
    }
    break;

  case PS_HTTPACTION_TYPE:
    {
      if ( b == ',' ) {
        Serial.print("HTTPACTION type is ");
        Serial.println(buffer);
        parseState = PS_HTTPACTION_RESULT;
        resetBuffer();
      }
    }
    break;

  case PS_HTTPACTION_RESULT:
    {
      if ( b == ',' ) {
        Serial.print("HTTPACTION result is ");
        Serial.println(buffer);
        parseState = PS_HTTPACTION_LENGTH;
        resetBuffer();
      }
    }
    break;

  case PS_HTTPACTION_LENGTH:
    {
      if ( b == '\n' ) {
        Serial.print("HTTPACTION length is ");
        Serial.println(buffer);
        
        // now request content
        GSM.print("AT+HTTPREAD=0,");
        GSM.println(buffer);
        
        parseState = PS_DETECT_MSG_TYPE;
        resetBuffer();
      }
    }
    break;

  case PS_HTTPREAD_LENGTH:
    {
      if ( b == '\n' ) {
        contentLength = atoi(buffer);
        Serial.print("HTTPREAD length is ");
        Serial.println(contentLength);
        
        Serial.print("HTTPREAD content: ");
        
        parseState = PS_HTTPREAD_CONTENT;
        resetBuffer();
      }
    }
    break;

  case PS_HTTPREAD_CONTENT:
    {
      // for this demo I'm just showing the content bytes in the serial monitor
      //Serial.write(b);
      //Serial.write("ass");
      //char s[11];
      //memcpy(s, b, sizeof b);
      //Serial.write(s);
      if(b == 'Y'){
        Serial.write("yes");
      }else{
        Serial.write("no");
      }
      contentLength--;
      
      if ( contentLength <= 0 ) {

        // all content bytes have now been read

        parseState = PS_DETECT_MSG_TYPE;
        resetBuffer();

        Serial.print("\n\n\n");

        actionState = AS_IDLE;
      }
    }
    break;
  }
}
