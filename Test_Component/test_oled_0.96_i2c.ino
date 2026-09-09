#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED not found! Check wiring/address.");
    while (1) delay(10);
  }

  Serial.println("OLED found! Displaying test text...");

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("SafeTrail OLED Test");

  display.setTextSize(2);
  display.setCursor(0, 20);
  display.println("Hello!");

  display.drawRect(0, 45, 128, 15, SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(5, 49);
  display.println("Status: OK");

  display.display();
}

void loop() {
  // static test
}
