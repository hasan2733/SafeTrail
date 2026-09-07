#define SOS_BUTTON  32
#define SAFE_BUTTON 33
#define MODE_BUTTON 25

void setup() {
  Serial.begin(115200);
  pinMode(SOS_BUTTON, INPUT_PULLDOWN);
  pinMode(SAFE_BUTTON, INPUT_PULLDOWN);
  pinMode(MODE_BUTTON, INPUT_PULLDOWN);
  Serial.println("Button test starting...");
}

void loop() {
  if (digitalRead(SOS_BUTTON) == HIGH) {
    Serial.println("SOS button pressed!");
    delay(300);
  }
  if (digitalRead(SAFE_BUTTON) == HIGH) {
    Serial.println("SAFE button pressed!");
    delay(300);
  }
  if (digitalRead(MODE_BUTTON) == HIGH) {
    Serial.println("MODE button pressed!");
    delay(300);
  }
}
