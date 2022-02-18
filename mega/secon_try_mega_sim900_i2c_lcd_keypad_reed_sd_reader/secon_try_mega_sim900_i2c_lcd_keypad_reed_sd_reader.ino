#include<SoftwareSerial.h>     
SoftwareSerial GSM(18,19);  //RX/TX  
//SoftwareSerial GSM(7,8);
//GSM specific settings
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
  Serial.println(msg);
  delay(waitMs);
  while(GSM.available()) {
    maryam(GSM.read());
  }
}
//GSM specific settings
 
const int  buttonPin = 2;    // the pin that the reed switch is attached to  
int buttonPushCounter = 0;   // counter for the number of times reed switch detects magnet presses
int buttonState = 0;         // current state of the reed switch

long previousMillisSd;
long intervalSd = 4000;

void setup()      
{      
  //start GSM  
  
  GSM.begin(19200);  
  GSM.println("i am working");
  Serial.begin(19200);
  Serial.println("i am alive");
  //start Serial  
  //Serial.begin(19200);
  //Serial.println("be dengel maryam sem");
  
  sendGSM("AT+SAPBR=3,1,\"APN\",\"internet\"");  
  sendGSM("AT+SAPBR=1,1",3000);
  sendGSM("AT+HTTPINIT");  
  sendGSM("AT+HTTPPARA=\"CID\",1");    
  
}    
  
void loop()      
{  
      
   buttonState = digitalRead(buttonPin);

   if (buttonState != HIGH) {
     buttonPushCounter++;
     GSM.println("on");
     GSM.print("number of button pushes: ");
     GSM.println(buttonPushCounter);
     delay(50);
   }

     //saves reed switch data to sdreader
  if(millis() - previousMillisSd > intervalSd){
    GSM.println("in if");
    //writeToSdCard();
    sendToServer();
    previousMillisSd = millis();
    
  }

  while(GSM.available()) {
    previousMillisSd = millis();
    maryam(GSM.read());
  }

  delay(50);
}

int sendToServer(){
   if(actionState == AS_IDLE){
      GSM.println("sending...");
      sprintf(url,"AT+HTTPPARA=\"URL\",\"http://www.nrwlpms.com/sim900/get_data.php?pre=%d\"",buttonPushCounter);
      sendGSM(url);
      sendGSM("AT+HTTPACTION=0");
      buttonPushCounter=0;
      delay(500);
      byte x = 0;
      while(GSM.available()) {
        x = GSM.read();

      }
              Serial.print("what i read");
      Serial.print(x);
      //actionState = AS_WAITING_FOR_RESPONSE; 
  }else{
    GSM.println(actionState);
  }
}

void maryam(byte b){

  Serial.println(b);
}

void parseATText(byte b) {

  buffer[pos++] = b;

  if ( pos >= sizeof(buffer) )
    resetBuffer(); // just to be safe

  /*
   // Detailed debugging
   Serial.println();
   Serial.print("state = ");
   Serial.println(state);
   Serial.print("b = ");
   Serial.println(b);
   Serial.print("pos = ");
   Serial.println(pos);
   Serial.print("buffer = ");
   Serial.println(buffer);*/

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
