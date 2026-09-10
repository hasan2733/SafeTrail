/* GPIO26 → 200Ω resistor → Transistor Base
Battery (+) → Motor (+)
Motor (-) → Transistor Collector
Transistor Emitter → GND (common with battery GND)
*/

#define VIBRATION_PIN 26

void setup() {
  Serial.begin(115200);
  pinMode(VIBRATION_PIN, OUTPUT);
  digitalWrite(VIBRATION_PIN, LOW);
  Serial.println("Vibration motor test starting...");
}

void loop() {
  Serial.println("Vibrating...");
  digitalWrite(VIBRATION_PIN, HIGH);
  delay(1000);

  digitalWrite(VIBRATION_PIN, LOW);
  Serial.println("Stopped.");
  delay(2000);
}
