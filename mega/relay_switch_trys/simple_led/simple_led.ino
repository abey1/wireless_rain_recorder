#define LED 10  // The pin the LED is connected to
void setup() {
  pinMode(LED, OUTPUT); // Declare the LED as an output
}

void loop() {
  digitalWrite(LED, HIGH); // Turn the LED on
  delay(5000);
  digitalWrite(LED, LOW);
  delay(5000);
}
