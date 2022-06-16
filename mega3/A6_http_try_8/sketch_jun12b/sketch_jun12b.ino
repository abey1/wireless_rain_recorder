char phone_no[] = "+251939793479";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(19200);
  delay(200);

  Serial.println("AT");
  delay(1000);
  Serial.print("ATD");
  Serial.println(phone_no);
  delay(8000);
  Serial.println("ATH");
}

void loop() {
  // put your main code here, to run repeatedly:

}
