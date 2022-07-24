
int fan = 2;
bool FANON = 0;
long intervalFanOn = 10000;
long intervalFanOff = 10000;
long previousMillisFanOn = millis();
long previousMillisFanOff = millis();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  
}

void faonOn(){
  digitalWrite(fan, HIGH);
  Serial.write('1');
}

void fanOff(){
  digitalWrite(fan, LOW);
  Serial.write('0');
}

void loop() {
  // put your main code here, to run repeatedly:
  if(!FANON){
      if(millis() - previousMillisFanOn > intervalFanOn) {
        faonOn(); 
        FANON = true;
        previousMillisFanOff = millis();
      }
  }
  if(FANON){
      if(millis() - previousMillisFanOff > intervalFanOff) {
        fanOff(); 
        FANON = false;
        previousMillisFanOn = millis();
      }
  }

}
