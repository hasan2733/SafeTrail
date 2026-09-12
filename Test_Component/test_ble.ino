#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE advertising test...");

  BLEDevice::init("SafeTrail-ESP32");
  BLEServer *pServer = BLEDevice::createServer();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->start();

  Serial.println("BLE advertising started. Search for 'SafeTrail-ESP32' on your phone's Bluetooth scanner.");
}

void loop() {
  // advertising runs in background, nothing needed here
  delay(1000);
}
