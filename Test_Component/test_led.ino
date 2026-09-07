#define LED 2

void setup() {
  pinMode(LED, OUTPUT);
}

void loop() {
  digitalWrite(LED, HIGH);  // ON
  delay(1000);

  digitalWrite(LED, LOW);   // OFF
  delay(1000);
}

// LED long leg (+ / anode) → resistor → GPIO 2
// LED short leg (− / cathode) → GND

// The external LED should blink ON → 1 second → OFF → 1 second → repeat.
