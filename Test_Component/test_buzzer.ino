#define BUZZER_PIN 27

void setup() {
  Serial.begin(115200);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  Serial.println("Buzzer test starting...");
}

void loop() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(150);
    digitalWrite(BUZZER_PIN, LOW);
    delay(150);
  }
  Serial.println("Beeped 3 times. Waiting 2s...");
  delay(2000);
}

// 3 tik sound
//tone(BUZZER_PIN, 2000); // 2kHz frequency - loudest resonance  
