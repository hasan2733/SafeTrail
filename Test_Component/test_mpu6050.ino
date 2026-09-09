#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

Adafruit_MPU6050 mpu;

void setup() {
  Serial.begin(115200);

  // ESP32 I2C
  Wire.begin(21, 22);

  Serial.println("MPU6050 Test");

  if (!mpu.begin()) {
    Serial.println("MPU6050 not found!");
    Serial.println("Check wiring and I2C address.");
    
    while (1) {
      delay(10);
    }
  }

  Serial.println("MPU6050 Found!");

  // Accelerometer range
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);

  // Gyroscope range
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);

  // Filter bandwidth
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  Serial.println("Sensor ready!");
  delay(1000);
}

void loop() {

  sensors_event_t a, g, temp;

  mpu.getEvent(&a, &g, &temp);

  Serial.println("-------------------------");

  // Accelerometer
  Serial.print("Acceleration X: ");
  Serial.print(a.acceleration.x);
  Serial.println(" m/s^2");

  Serial.print("Acceleration Y: ");
  Serial.print(a.acceleration.y);
  Serial.println(" m/s^2");

  Serial.print("Acceleration Z: ");
  Serial.print(a.acceleration.z);
  Serial.println(" m/s^2");

  // Gyroscope
  Serial.print("Gyro X: ");
  Serial.print(g.gyro.x);
  Serial.println(" rad/s");

  Serial.print("Gyro Y: ");
  Serial.print(g.gyro.y);
  Serial.println(" rad/s");

  Serial.print("Gyro Z: ");
  Serial.print(g.gyro.z);
  Serial.println(" rad/s");

  // Temperature
  Serial.print("Temperature: ");
  Serial.print(temp.temperature);
  Serial.println(" °C");

  delay(500);
}
