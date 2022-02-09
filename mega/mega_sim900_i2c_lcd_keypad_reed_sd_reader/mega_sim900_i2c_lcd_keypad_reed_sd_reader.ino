#include <Keypad.h>
#include <LiquidCrystal_I2C.h>

//import for clock
#include <Wire.h>
#include <DS3231.h>

//import for sdcard
#include <SPI.h>
#include <SD.h>

//import for GSM module
#include <SoftwareSerial.h>
SoftwareSerial GSM(7, 8); // RX, TX

//gsm specific variables and functions
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

char* dataBuf;
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

//gsm specific variables and functions

DS3231 clock;
RTCDateTime dt;

const int  buttonPin = 2;    // the pin that the reedswitch is attached to
int buttonPushCounter = 0;   // counter for the number of times reed switch detects magnet presses
int buttonState = 0;         // current state of the reed switch

const int ROW_NUM    = 4; // four rows
const int COLUMN_NUM = 4; // four columns

char keys[ROW_NUM][COLUMN_NUM] = {
  {'1','2','3', '.'},
  {'4','5','6', 'B'},
  {'7','8','9', 'C'},
  {'*','0','#', 'D'}
};

byte pin_rows[ROW_NUM] = {9, 8, 7, 6};      // connect to the row pinouts of the keypad
byte pin_column[COLUMN_NUM] = {5, 4, 3, 2}; // connect to the column pinouts of the keypad


Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );
LiquidCrystal_I2C lcd(0x27, 20, 4); // I2C address 0x27, 20 column and 4 rows

int cursorColumn = 0;

//buffers to hold and save user input for longitude and latitude
char longBuf[20];
char latBuf[20];

//this two variables holds and displays logitude and latitude
char longitude[20];
char latitude[20];

//this key is used to take in input from keypad to reset and brightn the lcd
//'D' to brightn lcd and 'B' to go back and reset are set 
char resetKey;

//previous millis is used to hold and save millis() to kim the lcd
long previousMillisLCD; //for backlight dim
long previousMillisSd; //to display seconds on lcd
//the interval for the lcd to stay showing light after user pressed 'D'
long intervalLCD = 30000;
long intervalSd = 4000;

//longIndex and latIndex are used to trace when user inputs value for longitude and latitude
//longDisplayIndex and latDisplayIndex are used to display latitude and longitude to lcd
int longIndex;
int longDisplayIndex;
int latIndex;
int latDisplayIndex;


int cc; //current cursor column
int cr; //current cursor row

//pin number on mega to access sd card
const int chipSelect = 53;

//next file to write reed switch data
File nextFile;

//number of files in SD card to be written
int noOfFile = 1;

//number of files to be sent to server by GSM
int noOfFileGSM = 1;

//variable to hold noOfFile changed to string for SD card
char sNoOfFile[10];

//variable to hold noOfFile changed to string for GSM module
char sNoOfFileGSM[10];

//timebuf to hold current time
char timeBuf[50];

//this function displays information to input logitude value for user
int printGetLongInfo(){
  cc = 0; //current cursor column
  cr = 0; //current cursor row
  lcd.clear();
  lcd.setCursor(cc,cr);
  lcd.print("A='.', esc='*'");
  cr++;
  lcd.setCursor(cc,cr);
  lcd.print("clr='C', save='#'");
  cr++;
  lcd.setCursor(cc,cr);
  lcd.print("Long : ");
  cc = cc + 7;
  lcd.setCursor(cc,cr);
}

//this function displays information to input latitude value for user
int printGetLatInfo(){
  cc = 0; //current cursor column
  cr = 0; //current cursor row
  lcd.clear();
  lcd.setCursor(cc,cr);
  lcd.print("A='.', esc='*'");
  cr++;
  lcd.setCursor(cc,cr);
  lcd.print("clr='C', save='#'");
  cr++;
  lcd.setCursor(cc,cr);
  lcd.print("Lat : ");
  cc = cc + 6;
  lcd.setCursor(cc,cr);
}

//this function takes in values from user to save it in longitude variable
int getLong(){
  //print get long info for user
  printGetLongInfo();
  char key;
  longIndex = 0;
  do{
    key = keypad.getKey();

    //if user is mistaken this will clear user input
    if(key && key=='C'){
      longIndex=0;
      longDisplayIndex=0;
      printGetLongInfo();
    }

    //save buffered value to permanent buffer
    if(key && key == '#'){
      //sizeof(longBuf)/sizeof(longBuf[0])
      for(int i = 0; i < longIndex; i++){
        longitude[i] = longBuf[i];
      }
      break;
    }

    //this function will save the input to buffer
    if(key && key != '*' && key != 'C'){
      lcd.print(key);
      longBuf[longIndex] = key;
      longIndex++;
      cc++;
      lcd.setCursor(cc,cr);  
    }
  }while(key != '*');

  //if the longIndex is not zero then assign the value to longDisaplayIndex for display purposes
  if(longIndex != 0){
      longDisplayIndex = longIndex;
  }
}

int getLat(){
  //print get lat info for user
  printGetLatInfo();
  char key;
  latIndex = 0;
  do{
    key = keypad.getKey();

    //clear
    if(key && key=='C'){
      latIndex=0;
      latDisplayIndex=0;
      printGetLatInfo();
    }

    //save buffered value to permanent buffer
    if(key && key == '#'){
      for(int i = 0; i < latIndex; i++){
        latitude[i] = latBuf[i];
      }
      break;
    }
    
    if(key && key != '*' && key != 'C'){
      lcd.print(key);
      latBuf[latIndex] = key;
      latIndex++;
      cc++;
      lcd.setCursor(cc,cr);  
    }
  }while(key != '*');

  if(latIndex != 0){
      latDisplayIndex = latIndex;
  }
}

void setup(){

  //begin GSM
  GSM.begin(19200);

  //initialization of gsm
  sendGSM("AT+SAPBR=3,1,\"APN\",\"internet\"");  
  sendGSM("AT+SAPBR=1,1",3000);
  sendGSM("AT+HTTPINIT");  
  sendGSM("AT+HTTPPARA=\"CID\",1");
  //initialization of gsm
  
  //begin clock
  clock.begin();

  //initialize Serial for debuging purposes
  Serial.begin(19200);
  
  // uncomment this to assign compiling date and time
  // Set sketch compiling time
  //clock.setDateTime(__DATE__, __TIME__);
  
  // initialize the lcd
  lcd.begin(); 
  
  //initialize sd card
  if(!SD.begin(chipSelect)){
    Serial.println("initialization failed. Things to check:");
    Serial.println("1. is a card inserted?");
    Serial.println("2. is your wiring correct?");
    Serial.println("3. did you change the chipSelect pin to match your shield or module?");
    Serial.println("Note: press reset or reopen this Serial Monitor after fixing your issue!");
    while (true);
  }
  
  lcd.backlight();
  getLong();
  getLat();

  //assign previousMillisLCD current time
  previousMillisLCD = millis(); 
  //assign previousMillisSd current time
  previousMillisSd = millis();


  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("(Lo:");
  lcd.setCursor(4,1);
  lcd.print(longitude);
  lcd.setCursor(4+longDisplayIndex,1);
  lcd.print(",La:");
  lcd.print(latitude);
  lcd.setCursor(4+longDisplayIndex+latDisplayIndex+4,1);
  lcd.print(")");
  lcd.setCursor(0,2);
  lcd.print("reed :");
  lcd.setCursor(0,3);
  lcd.print("memory :");
}

void loop(){
  //get key from user to setup values for longitude and latitude
  resetKey = keypad.getKey();

  //get date and time from clock
  dt = clock.getDateTime();

  //if user put 'B' as input go to setup()
  if(resetKey == 'B'){
    setup();
  }

  //wakeup show lcd
  if(resetKey == 'D'){
    lcd.backlight();
    previousMillisLCD = millis();
  }

   buttonState = digitalRead(buttonPin);

   if (buttonState != HIGH) {
     buttonPushCounter++;
     Serial.println("on");
     Serial.print("number of button pushes: ");
     Serial.println(buttonPushCounter);
   }


  //dims light of lcd after intervalLCD seconds
  if(millis() - previousMillisLCD > intervalLCD) {
    lcd.noBacklight(); 
  }

  //saves reed switch data to sdreader
  if(millis() - previousMillisSd > intervalSd){

    writeToSdCard();
    previousMillisSd = millis();
    
  }
  
  // Format the time and date and insert into the temporary buffer.
  
  snprintf(timeBuf, sizeof(timeBuf), "%04d-%02d-%02d %02d:%02d:%02d",
           dt.year, dt.month, dt.day,
           dt.hour, dt.minute, dt.second);

  //prints clock on first line
  lcd.setCursor(0,0);
  lcd.print(timeBuf);
  lcd.setCursor(7,2);
  lcd.print(buttonPushCounter);
}

void sendToServer(){
  int noOfFileGSMTemp = noOfFileGSM;
  
  while(noOfFileGSMTemp <= noOfFile){
    
    if(actionState == AS_IDLE){

      //change noOfFileGSMTemp to sNoOfFileGSM string
      itoa(noOfFileGSMTemp,sNoOfFileGSM,10);
      
      //assign next file after concatinating .TXT file to our string
      nextFile = SD.open(strcat(sNoOfFileGSM, ".TXT"));

      if (nextFile) {
        // read from the file until there's nothing else in it:
        while (nextFile.available()) {
          nextFile.read(dataBuf,10);
        }
        // close the file:
        nextFile.close();

        sprintf(url,"AT+HTTPPARA=\"URL\",\"http://www.nrwlpms.com/sim900/get_data.php?pre=%d\"",dataBuf);
        sendGSM(url);
        sendGSM("AT+HTTPACTION=0");
        actionState = AS_WAITING_FOR_RESPONSE;
      }
      
      noOfFileGSMTemp++;
    }
  }
}

int writeToSdCard() {
    //converting noOfFile which tracks number of files in sdcard to string which 
    //the string variable is sNoOfFile
    itoa(noOfFile,sNoOfFile,10);
    
    nextFile = SD.open(strcat(sNoOfFile, ".TXT"), FILE_WRITE);//first argument is filename
    if(nextFile){
      nextFile.print(timeBuf);
      nextFile.print("<->");
      nextFile.print(buttonPushCounter);
      
      nextFile.close();
      noOfFile++;
      buttonPushCounter=0;
      //readFromSdCard();
      //fileExists();
      //deleteFromSdCard();
      //fileExists();
    }else{
      // if the file didn't open, print an error:
      Serial.println("error opening file from read");
    }
    
}



int readFromSdCard(){
  // re-open the file for reading:
  //sNoOfFile is used here because it is concatinated to .txt file before in write
  nextFile = SD.open(sNoOfFile);
  Serial.println(sNoOfFile);
  if (nextFile) {
   
    // read from the file until there's nothing else in it:
    while (nextFile.available()) {
      Serial.write(nextFile.read());
    }
    // close the file:
    nextFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening file from read");
  }
}

int deleteFromSdCard(){
  if(SD.remove(sNoOfFile)){
    Serial.print(sNoOfFile);
    Serial.print("deleted");
  }else{
    Serial.print("can not delete");
    Serial.print(sNoOfFile);
  }
}

void fileExists(){
  if(SD.exists("*.TXT")){
    Serial.print("true");
  }else{
    Serial.print("false");
  }
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
