#include <Keypad.h>
#include <LiquidCrystal_I2C.h>

//GSM specific include
#include <SoftwareSerial.h> //software serial library for serial communication b/w arduino & mySerial

//initializing mySerial as our GSM SIM900A
SoftwareSerial mySerial(12, 13);//connect Tx pin of mySerial to pin 8 of arduino && Rx pin of mySerial to pin no 9 of arduino

char url[130];
char data[100];

//char buffer[80];
//byte pos = 0;

//import for clock
#include <Wire.h>
#include <DS3231.h>

//import for sdcard
#include <SPI.h>
#include <SD.h>

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

//number of files counter to be sent through GSM module
int noOfFileGSM = 1;

//variable to hold noOfFile changed to string
char sNoOfFile[10];

//timebuf to hold current time
char timeBuf[50];

//reset buffer everytime before reading
//void resetBuffer() {
  //memset(buffer, 0, sizeof(buffer));
  //pos = 0;
//}

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
  //-----------------------------------------------------
  //setup for the GSM SIM900A
  mySerial.begin(19200);
  
  //initialize Serial for debuging purposes
  Serial.begin(19200);

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

   // initialize http service
   mySerial.println("AT+HTTPINIT");
   delay(2000); 
   toSerial();
   //setup for the GSM SIM900A
   //-----------------------------------------------------
  
  //begin clock
  clock.begin();
  
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
      readFromSdCard();
      deleteFromSdCard();
      sendData();
    }else{
      // if the file didn't open, print an error:
      Serial.println("error opening file from read");
    }
    
}



int readFromSdCard(){

  //reset buffer
  //resetBuffer();
  // re-open the file for reading:
  //sNoOfFile is used here because it is concatinated to .txt file before in write
  nextFile = SD.open(sNoOfFile);
  Serial.println(sNoOfFile);
  if (nextFile) {
   
    // read from the file until there's nothing else in it:
    while (nextFile.available()) {
      Serial.write(nextFile.read());
      //buffer[pos++] = nextFile.read();
    }

    //Serial.write("--------------->");
    //Serial.write(buffer);
    
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


//send data through GSM module
void sendData() {
   // initialize http service
   mySerial.println("AT+HTTPINIT");
   //delay(2000); 
   toSerial();
 
   // set http param value
   memset(data, 0, 100);
   memset(url, 0, 130);

   sprintf(data,"lema");
   sprintf(url,"AT+HTTPPARA=\"URL\",\"http://www.nrwlpms.com/sim900/get_data.php?pre=%s\"",data);
   //mySerial.println(url);
   mySerial.println("AT+HTTPPARA=\"URL\",\"http://www.nrwlpms.com/sim900/get_data.php?pre=5\"");

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
