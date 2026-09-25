// Note this code will updated over time


/*
  =========================================================
  SafeTrail - Personal Safety & Emergency Response Wearable
  FULL FINAL CODE
  =========================================================

  Features in this build:
    - MPU6050 + DHT22 + GPS sensing
    - KNN risk classifier (5 features: accel, gyro, temp,
      humidity, distance-from-safe-zone)
    - Geofencing (haversine distance from a defined safe zone)
    - WiFi + ThingSpeak cloud dashboard (location + status)
    - SIM800L SMS — emergency fallback only
    - 3 buttons (SOS / SAFE / MODE), buzzer, vibration motor
    - Redesigned OLED UI (status bar + big state + risk bar)

  ---------------- WIRING SUMMARY ----------------
  MPU6050   SDA:21  SCL:22  VCC:3.3V  GND
  DHT22     DATA:4 (10k pull-up to VCC)  VCC:3.3V/5V  GND
  SSD1306   SDA:21  SCL:22  (addr 0x3C)
  Buzzer    I/O:27  VCC:3.3V/5V  GND        (passive -> tone())
  Buttons   SOS:32  SAFE:33  MODE:25        (3-pin modules, INPUT_PULLDOWN, active HIGH)
  Vibration GPIO26 -> 200ohm -> transistor base
            motor+ -> battery+, motor- -> transistor collector
            transistor emitter -> GND, flyback diode across motor
  GPS       RX2:16 (ESP RX2 <- GPS TX)  TX2:17 (ESP TX2 -> GPS RX)
  SIM800L   RX:13 (ESP RX1 <- SIM800L TX)  TX:14 (ESP TX1 -> SIM800L RX)
            !! Power SIM800L directly from the 3.7V battery, NOT ESP32 !!
            !! Common GND between battery and ESP32 is required !!

  ---------------- LIBRARIES NEEDED ----------------
    - Adafruit MPU6050, Adafruit Unified Sensor
    - Adafruit SSD1306, Adafruit GFX Library
    - DHT sensor library (Adafruit)
    - TinyGPSPlus
    - WiFi.h, HTTPClient.h (built-in with ESP32 board package)

  ---------------- BEFORE UPLOADING ----------------
    1. Set WIFI_SSID / WIFI_PASSWORD.
    2. Set THINGSPEAK_API_KEY.
    3. Set GUARDIAN_PHONE.
    4. Set SAFE_ZONE_LAT / SAFE_ZONE_LNG / SAFE_ZONE_RADIUS_M
       to your actual home/safe-zone coordinates and radius.
    5. Replace the placeholder trainData[]/trainLabels[] with
       your own collected, labeled readings for real accuracy.
*/

#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <TinyGPS++.h>

// ---------------- WiFi & Cloud config ----------------
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* THINGSPEAK_API_KEY = "YOUR_KEY";
const char* THINGSPEAK_URL = "http://api.thingspeak.com/update";
const unsigned long CLOUD_INTERVAL = 20000;  // ThingSpeak free tier needs >=15s

// ---------------- Guardian contact (SMS fallback) ----------------
const char* GUARDIAN_PHONE = "+8801747470294";

// ---------------- Geofence config ----------------
// Set these to your actual safe-zone center coordinates (e.g. home)
const double SAFE_ZONE_LAT = "2*.8**3";      // <-- replace with real latitude
const double SAFE_ZONE_LNG = "*0.41**";      // <-- replace with real longitude
const double SAFE_ZONE_RADIUS_M = 2000.0;  // safe zone radius in meters

// ---------------- Pin definitions ----------------
#define DHT_PIN 4
#define MODE_BUTTON 25
#define SAFE_BUTTON 33
#define SOS_BUTTON 32
#define BUZZER_PIN 27
#define VIBRATION_PIN 26

#define SIM_RX 13
#define SIM_TX 14
#define GPS_RX 16
#define GPS_TX 17

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C
#define DHTTYPE DHT22


int emergencyStreak = 0;
const int EMERGENCY_CONFIRM_COUNT = 5;

// ---------------- Objects ----------------
Adafruit_MPU6050 mpu;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
DHT dht(DHT_PIN, DHTTYPE);
TinyGPSPlus gps;
HardwareSerial gpsSerial(2);
HardwareSerial simSerial(1);

// ---------------- System state ----------------
enum SafetyState { SAFE,
                   CAUTION,
                   WARNING,
                   EMERGENCY };
enum OperatingMode { MODE_SAFE,
                     MODE_TRAVEL,
                     MODE_SILENT,
                     MODE_EMERGENCY,
                     MODE_TEST };

SafetyState currentState = SAFE;
OperatingMode currentMode = MODE_SAFE;
int riskScore = 0;

bool emergencyPending = false;
unsigned long emergencyPendingStart = 0;
const unsigned long CANCEL_WINDOW_MS = 5000;
bool emergencyAlreadySent = false;
bool geofenceAlertSent = false;

int sosPressCount = 0;
unsigned long lastSosPressTime = 0;
const unsigned long SOS_PRESS_WINDOW_MS = 2000;

unsigned long lastModeDebounce = 0;
unsigned long lastSafeDebounce = 0;
unsigned long lastSosDebounce = 0;
const unsigned long DEBOUNCE_MS = 300;

float temperature = 0, humidity = 0;
float lastLat = 0, lastLng = 0;
bool gpsFixValid = false;
bool wifiConnected = false;
bool insideSafeZone = true;
bool zoneStateInitialized = false;
double distanceFromSafeZone = 0;  // meters

// Geofence hysteresis prevents GPS jitter near the boundary
const double ZONE_EXIT_MARGIN_M = 100.0;
const double ZONE_ENTER_MARGIN_M = 100.0;

unsigned long lastDisplayUpdate = 0;
unsigned long lastDhtRead = 0;
unsigned long lastRiskUpdate = 0;
unsigned long lastCloudUpload = 0;
unsigned long lastWiFiRetry = 0;

// =========================================================
//  KNN RISK CLASSIFIER
// =========================================================
// Features: {accelMagnitude, gyroMagnitude, temperature, humidity, distanceFromSafeZone}
// Labels: 0=SAFE, 1=CAUTION, 2=WARNING, 3=EMERGENCY
//
// IMPORTANT: this is placeholder training data. Replace with your
// own collected + labeled readings (see the data-collection sketch
// from earlier) for real accuracy.

#define NUM_SAMPLES 25
#define NUM_FEATURES 5
#define K 5

const float featureMin[NUM_FEATURES] = { 0, 0, 0, 0, 0 };
const float featureMax[NUM_FEATURES] = { 30, 10, 50, 100, 500 };

const float trainData[NUM_SAMPLES][NUM_FEATURES] = {
  // --- SAFE (resting / normal walking, inside safe zone) ---
  { 9.8, 0.1, 28.0, 60, 20 },
  { 10.0, 0.2, 28.5, 58, 15 },
  { 9.9, 0.15, 29.0, 55, 30 },
  { 10.2, 0.3, 27.5, 62, 10 },
  { 9.7, 0.2, 28.0, 59, 25 },
  // --- CAUTION (brisk movement, near zone edge) ---
  { 13.0, 1.0, 29.0, 60, 150 },
  { 14.5, 1.2, 30.0, 58, 180 },
  { 12.8, 0.9, 29.5, 61, 160 },
  { 15.0, 1.4, 28.0, 57, 190 },
  { 13.5, 1.1, 29.8, 60, 170 },
  // --- WARNING (sudden movement, outside safe zone) ---
  { 19.0, 2.0, 30.0, 58, 300 },
  { 20.5, 2.3, 29.0, 60, 350 },
  { 18.5, 1.9, 31.0, 59, 320 },
  { 21.0, 2.5, 30.5, 57, 400 },
  { 19.5, 2.1, 29.5, 61, 330 },
  // --- EMERGENCY (fall impact, far from safe zone) ---
  { 26.0, 3.5, 29.0, 60, 450 },
  { 28.5, 4.0, 30.0, 58, 480 },
  { 27.0, 3.8, 29.5, 59, 460 },
  { 29.0, 4.2, 28.5, 61, 490 },
  { 26.5, 3.6, 30.0, 60, 470 },
  // --- EMERGENCY (fall impact even inside safe zone, e.g. at home) ---
  { 27.5, 3.9, 28.0, 60, 10 },
  { 29.5, 4.3, 27.5, 58, 5 },
  { 26.8, 3.7, 29.0, 61, 15 },
  { 28.0, 4.0, 28.5, 59, 8 },
  { 27.2, 3.8, 29.5, 60, 12 }
};

const int trainLabels[NUM_SAMPLES] = {
  0, 0, 0, 0, 0,
  1, 1, 1, 1, 1,
  2, 2, 2, 2, 2,
  3, 3, 3, 3, 3,
  3, 3, 3, 3, 3
};

float normalize(float value, int featureIdx) {
  float range = featureMax[featureIdx] - featureMin[featureIdx];
  if (range == 0) return 0;
  float v = (value - featureMin[featureIdx]) / range;
  if (v < 0) v = 0;
  if (v > 1) v = 1;
  return v;
}

float euclideanDistance(float* a, const float* b) {
  float sum = 0;
  for (int i = 0; i < NUM_FEATURES; i++) {
    float diff = a[i] - b[i];
    sum += diff * diff;
  }
  return sqrt(sum);
}

int knnPredict(float accelMag, float gyroMag, float temp, float hum, float distMeters) {
  float query[NUM_FEATURES] = {
    normalize(accelMag, 0),
    normalize(gyroMag, 1),
    normalize(temp, 2),
    normalize(hum, 3),
    normalize(distMeters, 4)
  };

  float normTrain[NUM_SAMPLES][NUM_FEATURES];
  for (int i = 0; i < NUM_SAMPLES; i++)
    for (int j = 0; j < NUM_FEATURES; j++)
      normTrain[i][j] = normalize(trainData[i][j], j);

  float distances[NUM_SAMPLES];
  int indices[NUM_SAMPLES];
  for (int i = 0; i < NUM_SAMPLES; i++) {
    distances[i] = euclideanDistance(query, normTrain[i]);
    indices[i] = i;
  }

  for (int i = 0; i < K; i++) {
    int minIdx = i;
    for (int j = i + 1; j < NUM_SAMPLES; j++)
      if (distances[indices[j]] < distances[indices[minIdx]]) minIdx = j;
    int t = indices[i];
    indices[i] = indices[minIdx];
    indices[minIdx] = t;
  }

  int voteCount[4] = { 0, 0, 0, 0 };
  for (int i = 0; i < K; i++) voteCount[trainLabels[indices[i]]]++;

  int bestLabel = 0, bestCount = voteCount[0];
  for (int i = 1; i < 4; i++)
    if (voteCount[i] > bestCount) {
      bestCount = voteCount[i];
      bestLabel = i;
    }
  return bestLabel;
}

// =========================================================
//  GEOFENCE (Haversine distance)
// =========================================================
double haversineDistance(double lat1, double lon1, double lat2, double lon2) {
  const double R = 6371000.0;  // Earth radius in meters
  double dLat = radians(lat2 - lat1);
  double dLon = radians(lon2 - lon1);
  double a = sin(dLat / 2) * sin(dLat / 2) + cos(radians(lat1)) * cos(radians(lat2)) * sin(dLon / 2) * sin(dLon / 2);
  double c = 2 * atan2(sqrt(a), sqrt(1 - a));
  return R * c;
}

void dumpRawAccelRegs() {
  Wire.beginTransmission(0x68);
  Wire.write(0x3B);

  byte error = Wire.endTransmission(false);

  if (error != 0) {
    Serial.printf("I2C write error: %d\n", error);
    return;
  }

  int bytesReceived = Wire.requestFrom(0x68, 6, true);

  Serial.printf("Bytes received: %d\n", bytesReceived);

  if (bytesReceived != 6) {
    Serial.println("ERROR: MPU6050 did not return 6 bytes");
    return;
  }

  int16_t rawX = (Wire.read() << 8) | Wire.read();
  int16_t rawY = (Wire.read() << 8) | Wire.read();
  int16_t rawZ = (Wire.read() << 8) | Wire.read();

  Serial.printf("RAW: X=%d Y=%d Z=%d\n", rawX, rawY, rawZ);
}

void checkGeofence() {
  // Do nothing until GPS has a valid fix
  if (!gpsFixValid) {
    return;
  }

  // Calculate distance from safe-zone center
  distanceFromSafeZone =
    haversineDistance(
      SAFE_ZONE_LAT,
      SAFE_ZONE_LNG,
      lastLat,
      lastLng);

  Serial.print("[GEOFENCE] Distance: ");
  Serial.print(distanceFromSafeZone, 1);
  Serial.println(" m");

  // ---------------------------------------------------------
  // First valid GPS reading
  // ---------------------------------------------------------
  if (!zoneStateInitialized) {

    insideSafeZone =
      (distanceFromSafeZone <= SAFE_ZONE_RADIUS_M);

    zoneStateInitialized = true;

    Serial.println(
      insideSafeZone
        ? "[GEOFENCE] Initial state: INSIDE"
        : "[GEOFENCE] Initial state: OUTSIDE");

    return;
  }

  // ---------------------------------------------------------
  // Currently INSIDE
  // Only declare OUTSIDE after crossing the extra margin.
  // This prevents GPS jitter from repeatedly changing state.
  // ---------------------------------------------------------
  if (insideSafeZone) {

    if (distanceFromSafeZone > SAFE_ZONE_RADIUS_M + ZONE_EXIT_MARGIN_M) {

      insideSafeZone = false;

      Serial.println("[GEOFENCE] User LEFT safe zone!");

      // Send exit alert only once
      if (!geofenceAlertSent) {

        geofenceAlertSent = true;

        String message =
          "SafeTrail ALERT: User has left the safe zone. "
          "Distance: "
          + String(distanceFromSafeZone, 0) + "m. Location: https://maps.google.com/?q=" + String(lastLat, 6) + "," + String(lastLng, 6);

        sendSMS(message);
      }
    }
  }

  // ---------------------------------------------------------
  // Currently OUTSIDE
  // Require the user to move sufficiently inside before
  // declaring the user back inside.
  // ---------------------------------------------------------
  else {

    if (distanceFromSafeZone < SAFE_ZONE_RADIUS_M - ZONE_ENTER_MARGIN_M) {

      insideSafeZone = true;

      geofenceAlertSent = false;

      Serial.println("[GEOFENCE] User RE-ENTERED safe zone.");
    }
  }
}

// =========================================================
//  SETUP
// =========================================================
void setup() {
  Serial.begin(115200);
  delay(1000);

  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
  simSerial.begin(9600, SERIAL_8N1, SIM_RX, SIM_TX);

  pinMode(MODE_BUTTON, INPUT_PULLDOWN);
  pinMode(SAFE_BUTTON, INPUT_PULLDOWN);
  pinMode(SOS_BUTTON, INPUT_PULLDOWN);
  pinMode(VIBRATION_PIN, OUTPUT);
  digitalWrite(VIBRATION_PIN, LOW);
  noTone(BUZZER_PIN);

  Wire.begin(21, 22);

  if (!mpu.begin()) {
    Serial.println("MPU6050 not found!");
  } else {
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  }

  dht.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED not found!");
  } else {
    showBootScreen();
  }

  connectWiFi();

  Serial.println("Initializing SIM800L...");
  delay(3000);
  simSerial.println("AT");
  delay(500);
  simSerial.println("AT+CMGF=1");
  delay(500);

  Serial.println("SafeTrail system initialized.");
}

// =========================================================
//  MAIN LOOP
// =========================================================
void loop() {
  readGPS();
  handleButtons();
  maintainWiFi();

  if (millis() - lastDhtRead > 2000) {
    lastDhtRead = millis();
    readDHT();
  }

  if (millis() - lastRiskUpdate > 300) {
    lastRiskUpdate = millis();
    checkGeofence();
    computeRiskScoreKNN();
  }

  if (wifiConnected && millis() - lastCloudUpload > CLOUD_INTERVAL) {
    lastCloudUpload = millis();
    uploadToCloud();
    readGPS();  // drain any GPS bytes buffered during the blocking HTTP call
  }

  if (millis() - lastDisplayUpdate > 400) {
    lastDisplayUpdate = millis();
    updateDisplay();
  }
}

// ---------------- WiFi ----------------
void connectWiFi() {
  Serial.println("[WiFi] Connecting...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
    delay(500);
    Serial.print(".");
  }

  wifiConnected = (WiFi.status() == WL_CONNECTED);
  Serial.println();
  Serial.println(wifiConnected ? "[WiFi] CONNECTED: " + WiFi.localIP().toString()
                               : "[WiFi] FAILED, will retry in background.");
}

void maintainWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    return;
  }
  wifiConnected = false;
  if (millis() - lastWiFiRetry > 15000) {
    lastWiFiRetry = millis();
    Serial.println("[WiFi] Reconnecting...");
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  }
}

// ---------------- Cloud upload (ThingSpeak) ----------------
void uploadToCloud() {
  if (WiFi.status() != WL_CONNECTED) {
    wifiConnected = false;
    return;
  }

  HTTPClient http;
  String url = String(THINGSPEAK_URL) + "?api_key=" + THINGSPEAK_API_KEY;
  url += "&field1=" + String(gpsFixValid ? lastLat : 0, 6);
  url += "&field2=" + String(gpsFixValid ? lastLng : 0, 6);
  url += "&field3=" + String(riskScore);
  url += "&field4=" + String(temperature, 1);
  url += "&field5=" + String(humidity, 0);
  url += "&field6=" + String((int)currentState);

  http.begin(url);
  http.setTimeout(10000);
  int httpCode = http.GET();

  Serial.print("[Cloud] ");
  Serial.println(httpCode > 0 ? ("Upload OK, code " + String(httpCode))
                              : ("Upload failed: " + http.errorToString(httpCode)));
  http.end();
}

// ---------------- GPS ----------------
void readGPS() {
  while (gpsSerial.available() > 0) {
    if (gps.encode(gpsSerial.read())) {
      if (gps.location.isValid()) {
        lastLat = gps.location.lat();
        lastLng = gps.location.lng();
        gpsFixValid = true;
      }
    }
  }
}

// ---------------- DHT22 ----------------
void readDHT() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (!isnan(h) && !isnan(t)) {
    humidity = h;
    temperature = t;
  }
}

void computeRiskScoreKNN() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float accelMag = sqrt(a.acceleration.x * a.acceleration.x + a.acceleration.y * a.acceleration.y + a.acceleration.z * a.acceleration.z);
  float gyroMag = sqrt(g.gyro.x * g.gyro.x + g.gyro.y * g.gyro.y + g.gyro.z * g.gyro.z);

  float distMeters = gpsFixValid ? (float)distanceFromSafeZone : 0;

  // DEBUG
  Serial.print("ax=");
  Serial.print(a.acceleration.x);
  Serial.print(" ay=");
  Serial.print(a.acceleration.y);
  Serial.print(" az=");
  Serial.print(a.acceleration.z);
  Serial.print(" | gx=");
  Serial.print(g.gyro.x);
  Serial.print(" gy=");
  Serial.print(g.gyro.y);
  Serial.print(" gz=");
  Serial.println(g.gyro.z);

  dumpRawAccelRegs();

  int predictedLabel = knnPredict(accelMag, gyroMag, temperature, humidity, distMeters);

  if (predictedLabel == 3) {
    emergencyStreak++;
  } else {
    emergencyStreak = 0;
  }

  if (emergencyStreak >= EMERGENCY_CONFIRM_COUNT) {
    currentState = EMERGENCY;
    riskScore = 99;
    if (!emergencyAlreadySent) {
      triggerEmergency("KNN classifier confirmed EMERGENCY (sustained)");
    }
  } else {
    currentState = (SafetyState)predictedLabel;
    riskScore = predictedLabel * 33;
  }
}

// ---------------- Buttons ----------------
void handleButtons() {
  if (digitalRead(MODE_BUTTON) == HIGH && millis() - lastModeDebounce > DEBOUNCE_MS) {
    lastModeDebounce = millis();
    cycleMode();
  }
  if (digitalRead(SAFE_BUTTON) == HIGH && millis() - lastSafeDebounce > DEBOUNCE_MS) {
    lastSafeDebounce = millis();
    resolveEmergency();
  }
  if (digitalRead(SOS_BUTTON) == HIGH && millis() - lastSosDebounce > DEBOUNCE_MS) {
    lastSosDebounce = millis();
    handleSosPress();
  }
  if (emergencyPending && millis() - emergencyPendingStart > CANCEL_WINDOW_MS) {
    emergencyPending = false;
    triggerEmergency("Manual SOS (3x press)");
  }
}

void cycleMode() {
  currentMode = (OperatingMode)((currentMode + 1) % 5);
  Serial.print("Mode changed to: ");
  Serial.println(currentMode);
}

void handleSosPress() {
  if (millis() - lastSosPressTime > SOS_PRESS_WINDOW_MS) sosPressCount = 0;
  sosPressCount++;
  lastSosPressTime = millis();

  if (sosPressCount >= 3) {
    sosPressCount = 0;
    emergencyPending = true;
    emergencyPendingStart = millis();
    Serial.println("SOS armed! 5-second cancel window started.");
    vibrate(200);
  }
}

void resolveEmergency() {
  if (currentState == EMERGENCY || emergencyPending) {
    emergencyPending = false;
    emergencyAlreadySent = false;
    currentState = SAFE;
    riskScore = 0;
    noTone(BUZZER_PIN);
    digitalWrite(VIBRATION_PIN, LOW);
    Serial.println("Emergency resolved: I'M SAFE confirmed.");
    sendSMS("SafeTrail: User confirmed I'M SAFE. Situation resolved.");
  }
}

// ---------------- Emergency handling ----------------
void triggerEmergency(const char* reason) {
  if (emergencyAlreadySent) return;
  emergencyAlreadySent = true;
  currentState = EMERGENCY;

  Serial.print("EMERGENCY TRIGGERED: ");
  Serial.println(reason);

  if (wifiConnected) uploadToCloud();
  sendEmergencyAlert();
}

void sendEmergencyAlert() {
  String msg = "SafeTrail ALERT: Emergency detected. ";
  if (gpsFixValid) {
    msg += "Location: https://maps.google.com/?q=" + String(lastLat, 6) + "," + String(lastLng, 6);
  } else {
    msg += "Location: GPS fix not available.";
  }
  //sendSMS(msg);
  alert();
}

void alert() {
  if (currentMode == MODE_SILENT) {
    vibrate(1500);
  } else {
    tone(BUZZER_PIN, 2000);
    vibrate(1500);
    delay(1500);
    noTone(BUZZER_PIN);
  }
}

void vibrate(int durationMs) {
  digitalWrite(VIBRATION_PIN, HIGH);
  delay(durationMs);
  digitalWrite(VIBRATION_PIN, LOW);
}

// ---------------- SIM800L SMS ----------------
void sendSMS(String message) {
  Serial.print("Sending SMS: ");
  Serial.println(message);

  simSerial.println("AT+CMGF=1");
  delay(300);
  simSerial.print("AT+CMGS=\"");
  simSerial.print(GUARDIAN_PHONE);
  simSerial.println("\"");
  delay(300);
  simSerial.print(message);
  delay(200);
  simSerial.write(26);  // Ctrl+Z
  delay(3000);
}

// =========================================================
//  OLED UI
// =========================================================
void showBootScreen() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(10, 15);
  display.println("SafeTrail");
  display.setTextSize(1);
  display.setCursor(20, 40);
  display.println("Booting system...");
  display.drawRoundRect(0, 0, 128, 64, 6, SSD1306_WHITE);
  display.display();
}

// Small icon helpers -------------------------------------------------
void drawWifiIcon(int x, int y, bool connected) {
  if (connected) {
    display.drawLine(x, y + 6, x + 2, y + 4, SSD1306_WHITE);
    display.drawLine(x + 2, y + 4, x + 4, y + 6, SSD1306_WHITE);
    display.drawLine(x + 1, y + 3, x + 3, y + 1, SSD1306_WHITE);
    display.drawLine(x + 3, y + 1, x + 5, y + 3, SSD1306_WHITE);
    display.fillCircle(x + 3, y + 7, 1, SSD1306_WHITE);
  } else {
    display.drawLine(x, y, x + 6, y + 8, SSD1306_WHITE);
    display.drawLine(x, y + 8, x + 6, y, SSD1306_WHITE);
  }
}

void drawGpsIcon(int x, int y, bool fixed) {
  display.drawCircle(x + 3, y + 3, 3, SSD1306_WHITE);
  if (fixed) display.fillCircle(x + 3, y + 3, 1, SSD1306_WHITE);
}

void drawSimIcon(int x, int y, bool ok) {
  display.drawRect(x, y, 6, 8, SSD1306_WHITE);
  if (ok) display.fillRect(x + 1, y + 1, 4, 6, SSD1306_WHITE);
}

// Main status screen ---------------------------------------------------
void updateDisplay() {
  display.clearDisplay();

  // ---- Top status bar ----
  display.drawFastHLine(0, 10, 128, SSD1306_WHITE);
  drawWifiIcon(2, 1, wifiConnected);
  drawGpsIcon(16, 1, gpsFixValid);
  drawSimIcon(30, 1, true);  // SIM presence assumed once initialized

  display.setTextSize(1);
  display.setCursor(45, 1);
  display.print(insideSafeZone ? "ZONE:IN" : "ZONE:OUT");

  display.setCursor(100, 1);
  display.print(currentMode == MODE_SILENT ? "SIL" : currentMode == MODE_TRAVEL ? "TRV"
                                                   : currentMode == MODE_TEST   ? "TST"
                                                                                : "STD");

  // ---- Big state label ----
  display.setTextSize(2);
  display.setCursor(4, 15);
  display.println(stateToString(currentState));

  // ---- Risk bar ----
  display.drawRect(4, 34, 120, 10, SSD1306_WHITE);
  int fillWidth = map(riskScore, 0, 100, 0, 118);
  display.fillRect(5, 35, fillWidth, 8, SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(6, 36);
  display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
  display.print(riskScore);
  display.print("%");
  display.setTextColor(SSD1306_WHITE);

  // ---- Bottom info line ----
  display.setCursor(0, 48);
  display.print("T:");
  display.print(temperature, 1);
  display.print("C H:");
  display.print(humidity, 0);
  display.print("%");

  display.setCursor(0, 57);
  if (gpsFixValid) {
    display.print(lastLat, 3);
    display.print(",");
    display.print(lastLng, 3);
  } else {
    display.print("GPS: searching...");
  }

  if (emergencyPending) {
    display.fillRect(0, 0, 128, 10, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    display.setCursor(2, 1);
    display.print("SOS ARMED - press SAFE");
    display.setTextColor(SSD1306_WHITE);
  }

  display.display();
}

String stateToString(SafetyState s) {
    switch (s) {
      case SAFE: return "SAFE";
      case CAUTION: return "CAUTION";
      case WARNING: return "WARNING";
      case EMERGENCY: return "EMERGENCY";
    }
    return "UNKNOWN";
  }
