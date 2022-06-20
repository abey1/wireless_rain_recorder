/*
  I2C Master Demo
  i2c-master-demo.ino
  Demonstrate use of I2C bus
  Master sends character and gets reply from Slave
  DroneBot Workshop 2019
  https://dronebotworkshop.com
*/

// Include Arduino Wire library for I2C
#include <Wire.h>
#include <Keypad.h>
#include <DS3231.h>
#include <SPI.h>
#include <SD.h>
DS3231 clock;
RTCDateTime dt;
#include <LiquidCrystal_I2C.h>


char longFile[9] = "long.TXT"; 
char latFile[8] = "lat.TXT"; 

LiquidCrystal_I2C lcd(0x27, 20, 4);

// Define Slave I2C Address
#define SLAVE_ADDR 9

// Define Slave answer size
#define ANSWERSIZE 5

#define START_SENDING 0

#define IDLEE "idlee"
#define BUSYY "busyy"



File nextFile;

char buffer[2000];

int noOfFile = 1;

byte pos = 0;

int upToNoOfFile = 1;

//variable to hold noOfFile changed to string
char sNoOfFile[10];

const int  buttonPin = 2;    // the pin that the reedswitch is attached to
int buttonPushCounter = 0; // counter for the number of times reed switch detects magnet presses
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
long sendingGap;
//the interval for the lcd to stay showing light after user pressed 'D'
long intervalLCD = 60000;
long intervalSd = 900000;
long sendingInterval = 120000;

//longIndex and latIndex are used to trace when user inputs value for longitude and latitude
//longDisplayIndex and latDisplayIndex are used to display latitude and longitude to lcd
int longIndex;
int longDisplayIndex;
int latIndex;
int latDisplayIndex;

int cc; //current cursor columnchipSelect
int cr; //current cursor row

//pin number on mega to access sd card
const int chipSelect = 53;

//timebuf to hold current time
char timeBuf[50];

//
int noRainCounter = 0;

//
int startSending = 0;


void resetBuffer() {
  memset(buffer, 0, sizeof(buffer));
  pos = 0;
}

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

int printLongLatFound(char* message){
  cc = 0; //current cursor column
  cr = 0; //current cursor row
  lcd.clear();
  lcd.setCursor(cc, cr);
  lcd.print(message);
  lcd.setCursor(cc, cr+1);
  lcd.print("C : yes, B : no");
}

int printLoadingFile(char* message){
  cc = 0; //current cursor column
  cr = 0; //current cursor row
  lcd.clear();
  lcd.setCursor(cc, cr);
  lcd.print(message);
}

int printMemoryResetMessage(char* message){
  cc = 0; //current cursor column
  cr = 0; //current cursor row
  lcd.clear();
  lcd.setCursor(cc, cr);
  lcd.print(message);
  lcd.setCursor(cc, cr+1);
  lcd.print("C : yes, B : no");
}

int printCountDown(int val){
  cc = 2;
  cr = 2;
  lcd.setCursor(cc, cr);
  lcd.print(val);
}

int formatMemory(){
  char fileToBeDeleted[10];
  for(int i = 1; i <= noOfFile; i++){
     itoa(i,fileToBeDeleted,10);
    strcat(fileToBeDeleted, ".TXT");
    if(SD.remove(fileToBeDeleted)){
      Serial.print(fileToBeDeleted);
      Serial.print(" ");
      Serial.println("deleted");
    }
  }
  noOfFile = 1;
}

int resetMemory(){
  char key;
  int contWithCurrent = 1;
  printMemoryResetMessage("Reset Memory?");
  long now = millis();
  long waitInterval = 20000;
  //wait user input for 20 seconds
  do{
    key = keypad.getKey();
       if(key && key=='C'){
          contWithCurrent = 0;
          break;
       }
       if(key && key=='B'){
          break;
       }
  }while(millis() - now < waitInterval);

  if(contWithCurrent == 0){
    formatMemory();
  }
}

int readLong(){
  //reset position
  byte posi = 0;
  
  //declair long and lat file
  File file;

  //open longFile
  file = SD.open(longFile);

  //if longFile exists read it and save the value in longitude
  if (file) {
    // read from the file until there's nothing else in it:
    while (file.available() && posi < 35) {
      longitude[posi++] = file.read();
    }
    // close the file:
    file.close();
  }
  longDisplayIndex = posi;
}

int writeLong(char* fileName){
  //remove the file if it exists
  SD.remove(fileName);
  
  File longLatFile = SD.open(fileName, FILE_WRITE);//first argument is filename
    if(longLatFile){
      longLatFile.print(longitude);
      longLatFile.close();
    }
}

//this function takes in values from user to save it in longitude variable
int getLong(){
  char key;
  int contWithCurrent = 1;
  if(detectLongLat(longFile)){
    readLong();
    printLongLatFound("long found cont?");
    long now = millis();
    long waitInterval = 20000;
    int val = 6;
    do{
      key = keypad.getKey();
       if(key && key=='B'){
          contWithCurrent = 0;
          break;
       }
       if(key && key=='C'){
          break;
       }
    }while(millis() - now < waitInterval);
  }else{
    contWithCurrent = 0;
  }

  if(contWithCurrent == 0){
    //print get long info for user
    printGetLongInfo();
    
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
        //write longitude to file
        writeLong(longFile);
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
}

int detectLongLat(char* fileName){
  File file;
  file = SD.open(fileName);
  if(file){
    return 1;
  }else{
    return 0;
  }
}

int readLat(){
  //reset position
  byte posi = 0;
  
  //declair long and lat file
  File file;

  //open longFile
  file = SD.open(latFile);

  //if longFile exists read it and save the value in longitude
  if (file) {
    // read from the file until there's nothing else in it:
    while (file.available() && posi < 35) {
      latitude[posi++] = file.read();
    }
    // close the file:
    file.close();
  }
  latDisplayIndex = posi;
}

int writeLat(char* fileName){
  //remove the file if it exists
  SD.remove(fileName);
  
  File longLatFile = SD.open(fileName, FILE_WRITE);//first argument is filename
    if(longLatFile){
      longLatFile.print(latitude);
      longLatFile.close();
    }
}

int getLat(){
  char key;
  int contWithCurrent = 1;
  if(detectLongLat(latFile)){
    readLat();
    printLongLatFound("lat found cont?");
    long now = millis();
    long waitInterval = 20000;
    int val = 6;
    do{
      key = keypad.getKey();
       if(key && key=='B'){
          contWithCurrent = 0;
          break;
       }
       if(key && key=='C'){
          break;
       }
    }while(millis() - now < waitInterval);
  }else{
    contWithCurrent = 0;
  }

  if(contWithCurrent == 0){
    //print get lat info for user
    printGetLatInfo();
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
        //write latitude to file
        writeLat(latFile);
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
}

int getLastNoOfFile(){
  printLoadingFile("loading file...");
  //this function will read and continue from old saved noOfFile if power is out
  int tempNoOfFile = 1;
  char sTempNoOfFile[10];
  //a stands for available
  bool a = false;
  do{
    itoa(tempNoOfFile,sTempNoOfFile,10);
    
    strcat(sTempNoOfFile, ".TXT");
    // re-open the file for reading:
    //sNoOfFile is used here because it is concatinated to .txt file before in write
    nextFile = SD.open(sTempNoOfFile);
    if(nextFile){
      a = true;
      Serial.print("found");
      Serial.println(sTempNoOfFile);
      nextFile.close();
      tempNoOfFile++;
      delay(100);
    }else{
      a = false;
    }
    
  }while(a);
  noOfFile = tempNoOfFile;
}

int printLCDessentials(){
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

void setup() {
  
  // Initialize I2C communications as Master
  Wire.begin();
  
  // Setup serial monitor
  Serial.begin(9600);
  Serial.println("I2C Master Demonstration");
  lcd.begin(); 
  lcd.backlight();

  //begin clock
  clock.begin();
  // uncomment this to assign compiling date and time
  // Set sketch compiling time
  clock.setDateTime(__DATE__, __TIME__);

  //initialize sd card
  if(!SD.begin(chipSelect)){
    Serial.println("initialization failed. Things to check:");
    Serial.println("1. is a card inserted?");
    Serial.println("2. is your wiring correct?");
    Serial.println("3. did you change the chipSelect pin to match your shield or module?");
    Serial.println("Note: press reset or reopen this Serial Monitor after fixing your issue!");
    while (true);
  }

  
  getLong();
  getLat();

  //assign previousMillisLCD current time
  previousMillisLCD = millis(); 
  //assign previousMillisSd current time
  sendingGap = millis();
  previousMillisSd = millis();
  getLastNoOfFile();
  resetMemory();
  lcd.clear();
  printLCDessentials();
}

void loop() {

  //get key from keypad
  resetKey = keypad.getKey();
  
  //get date and time from clock
  dt = clock.getDateTime();

  //wakeup show lcd
  if(resetKey == 'D'){
    lcd.backlight();
    previousMillisLCD = millis();
  }

  //read from reed switch
  buttonState = digitalRead(buttonPin);

  //tipping bucket sd saver
  if (buttonState != HIGH) {
     buttonPushCounter++;
     //save to file
     writeToSdCard('1');
     delay(5);
     //reset default saver
     previousMillisSd = millis();
     //subtract one from no-rain counter
     if(noRainCounter > 0){
      noRainCounter--;
     }
     //add one to the number of files
     noOfFile++;
     
     Serial.println("on");
     Serial.print("number of button pushes: ");
     Serial.println(buttonPushCounter);
   }

   //dims light of lcd after intervalLCD seconds
  if(millis() - previousMillisLCD > intervalLCD) {
    lcd.noBacklight(); 
  }

  //periodically saves value of 0 to sdreader if 
  //no rain appeared for some time
  if(millis() - previousMillisSd > intervalSd){
       writeToSdCard('0');
       //add one to noRainCouter
       noRainCounter++;
       //if noRainCounter reaches certain value
       //start sending
       if(noRainCounter == 5){
         //reset noRainCounter to 0
         noRainCounter = 0;
         //set flag to start sending to 1 
         startSending = 1;
       }else{
         noOfFile++;
       }
    //reset default sending time
    previousMillisSd = millis();
  }

  
//  Serial.println(DATA);
//  int dataSize = strlen(DATA);
//  
//  resetBuffer();
//  
//  for(int i = 0; i < dataSize; i++){
//    buffer[pos++] = DATA[i];
//  }

  //8888888888888888888888888888888

  //dims light of lcd after intervalLCD seconds
  if(millis() - sendingGap > sendingInterval) {
    if(startSending == 1){
      if(isSlaveIdle()){
        readFromSdCard();
        sendDataToSlave();
      }
    }
    sendingGap = millis(); 
  }
  
  

  if(noOfFile == 1){
    startSending = 0;
  }
  //78888888888888888888888888888

  snprintf(timeBuf, sizeof(timeBuf), "%04d-%02d-%02d_%02d:%02d:%02d",
           dt.year, dt.month, dt.day,
           dt.hour, dt.minute, dt.second);

  //prints clock on first line
  lcd.setCursor(0,0);
  lcd.print(timeBuf);
  lcd.setCursor(7,2);
  lcd.print(buttonPushCounter);
  lcd.setCursor(9,3);
  lcd.print(sNoOfFile);
  
  //delay(500);
  
  //receiveDataFromSlave();
  
  //readFromSdCard();
  //delay(500);
  //writeToSdCard();
  //delay(500);
  //readFromSdCard();
  //writeDataToSlave(START_SENDING);
  //readFromSdCard();
  //receiveDataFromSlave();
  //Serial.println("continue work");
  //while(1);
  //writeDataToSlave();
    
  //receiveDataFromSlave();
}

int writeToSdCard(char amount){
  if(noOfFile == 0){
      noOfFile = 1;
  }
  itoa(noOfFile,sNoOfFile,10);
  strcat(sNoOfFile, ".TXT");
  SD.remove(sNoOfFile);
  nextFile = SD.open(sNoOfFile, FILE_WRITE);
  if(nextFile){
      Serial.println(timeBuf);
      nextFile.print(timeBuf);
      nextFile.print("|");
      nextFile.print(amount);
      nextFile.print(",");
      nextFile.close();
  }
  lcd.setCursor(9,3);
  lcd.print("          ");
}

void sendDataToSlave(){
  char temp[32];
  Serial.println("");
  for(int i = 0; i <= pos; i+=32){  
    int k = 0;
    for(int j = i; j < i+32; j++){
      temp[k] = buffer[j];
      k++;
    }
    Serial.println(temp);
    Wire.beginTransmission(SLAVE_ADDR);
    Wire.write(temp);
    Wire.endTransmission();
    delay(500);
  }
}

int writeDataToSlave(byte order){
  Serial.println("Write data to slave");
  
  // Write a charatre to the Slave
  Wire.beginTransmission(SLAVE_ADDR);
  Wire.write("55,217,2022-06-02_09:11:23|0,2022-06-02_09:11:17|0,2022-06-02_09:11:12|0,2022-06-02_09:04:25|0,2022-06-02_06:48:22|0,2022-06-02_06:48:16|0,2022-06-02_06:48:11|0,2022-06-02_06:48:05|0,2022-06-02_06:42:57|0,2022-06-02_06:42:51|0");
  Wire.endTransmission();
}

bool isSlaveIdle(){
  bool idle = true;
  
  Serial.println("Receive data");
  
  // Read response from Slave
  // Read back 5 characters
  Wire.requestFrom(SLAVE_ADDR,ANSWERSIZE);
  
  // Add characters to string
  String response = "";
  while (Wire.available()) {
      char b = Wire.read();
      response += b;
  } 
  
  // Print to Serial Monitor
  Serial.println(response);
  if(response == BUSYY){
    //deleteFiles();
    idle = false;
  }

  return idle;
}

void deleteFiles(){
  Serial.println("file deletion...");
}

int readFromSdCard(){

  //initialize sd card
  if(!SD.begin(chipSelect)){
    Serial.println("initialization failed. Things to check:");
    Serial.println("1. is a card inserted?");
    Serial.println("2. is your wiring correct?");
    Serial.println("3. did you change the chipSelect pin to match your shield or module?");
    Serial.println("Note: press reset or reopen this Serial Monitor after fixing your issue!");
  }else{
    resetBuffer();
    
    //read long file
    nextFile = SD.open(longFile);
    if(nextFile){
      while (nextFile.available()) {
          //Serial.write(nextFile.read());
          buffer[pos++] = nextFile.read();
      }
      buffer[pos++] = ',';
      nextFile.close();
    }
    
    //read lat file
    nextFile = SD.open(latFile);
    if(nextFile){
      while (nextFile.available()) {
          //Serial.write(nextFile.read());
          buffer[pos++] = nextFile.read();
      }
      buffer[pos++] = ',';
      nextFile.close();
    }
  
    int range;
    
    if(noOfFile > 5)
      range = 5;
    else
      range = noOfFile;
      
    int limit = noOfFile - range;
    
    for(upToNoOfFile = noOfFile; upToNoOfFile > limit; upToNoOfFile--){
      
      itoa(upToNoOfFile,sNoOfFile,10);
      strcat(sNoOfFile, ".TXT");
      nextFile = SD.open(sNoOfFile);
      Serial.println(sNoOfFile);
      
      if (nextFile) {
        while (nextFile.available()) {
          buffer[pos++] = nextFile.read();
        }
  
        nextFile.close();
      } else {
        Serial.println("error opening file from read");
      }
    }
    Serial.write("--------------->");
    Serial.write(buffer);
    deleteFromSdCardByInterval(range);
  }
}

int deleteFromSdCardByInterval(int range){
  Serial.print("upToNoOfFile = ");
  Serial.print(upToNoOfFile);
  for(int tempNoOfFile = upToNoOfFile+1; tempNoOfFile <= noOfFile; tempNoOfFile++){
    
    itoa(tempNoOfFile,sNoOfFile,10);
    strcat(sNoOfFile, ".TXT");
    Serial.print("deleting");
    Serial.println(sNoOfFile);
    SD.remove(sNoOfFile);
  }
  noOfFile -= range;
}
