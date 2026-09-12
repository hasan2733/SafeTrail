#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  delay(1000);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  Serial.println("WiFi scan test starting...");
}

void loop() {
  Serial.println("Scanning...");
  int n = WiFi.scanNetworks();

  if (n == 0) {
    Serial.println("No networks found. Check antenna/board.");
  } else {
    Serial.print(n);
    Serial.println(" networks found:");
    for (int i = 0; i < n; i++) {
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.println(" dBm)");
    }
  }

  Serial.println("----------------------------");
  delay(5000);
}
