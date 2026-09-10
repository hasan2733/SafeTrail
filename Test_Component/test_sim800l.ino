#include <HardwareSerial.h>

HardwareSerial simSerial(1);

const char* GUARDIAN_PHONE = "+8801420420420; // valid phone number

void setup() {
  Serial.begin(115200);
  simSerial.begin(9600, SERIAL_8N1, 13, 14);

  Serial.println("Waiting 15 sec for SIM800L boot...");
  delay(15000);

  Serial.println(">> AT");
  simSerial.println("AT");
  delay(1000);
  printResponse();

  Serial.println(">> AT+CSQ (signal quality)");
  simSerial.println("AT+CSQ");
  delay(1000);
  printResponse();

  Serial.println(">> AT+CREG? (network registration)");
  simSerial.println("AT+CREG?");
  delay(1000);
  printResponse();

  Serial.println("Setup done. Type 'send' in Serial Monitor to test SMS.");
}

void loop() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd == "send") {
      sendTestSMS();
    } else {
      simSerial.println(cmd); // manual AT command pass-through
      delay(500);
      printResponse();
    }
  }
}

void printResponse() {
  unsigned long start = millis();
  while (millis() - start < 2000) {
    if (simSerial.available()) {
      Serial.write(simSerial.read());
    }
  }
  Serial.println();
}

void sendTestSMS() {
  Serial.println("Sending test SMS...");
  simSerial.println("AT+CMGF=1");
  delay(500);
  printResponse();

  simSerial.print("AT+CMGS=\"");
  simSerial.print(GUARDIAN_PHONE);
  simSerial.println("\"");
  delay(500);

  simSerial.print("SafeTrail test message.");
  delay(500);
  simSerial.write(26); // Ctrl+Z
  delay(5000);
  printResponse();
}
