#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <MPU6050.h> 

const char* ssid = "gucheng111";
const char* password = "Zz245680!!";
const char* serverUrl = "http://192.168.1.214:5000/data";

MPU6050 mpu;

const int sampleInterval = 20;  // 50Hz
const int batchSize = 50;       // Send every 50 samples (~1 second)

unsigned long lastSampleTime = 0;

String dataBuffer = "";  // Holds samples in JSON array
int bufferCount = 0;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  Wire.begin(23, 22);
  mpu.initialize();

  if (!mpu.testConnection()) {
    Serial.println("MPU6050 connection failed");
    while (1);
  }

  dataBuffer = "[";  // Start JSON array
}

void loop() {
  unsigned long now = millis();

  // Sample at 50Hz
  if (now - lastSampleTime >= sampleInterval) {
    lastSampleTime = now;

    int16_t ax, ay, az, gx, gy, gz;
    mpu.getAcceleration(&ax, &ay, &az);
    mpu.getRotation(&gx, &gy, &gz);

    String sample = "{";
    sample += "\"accel_x\":" + String(ax) + ",";
    sample += "\"accel_y\":" + String(ay) + ",";
    sample += "\"accel_z\":" + String(az) + ",";
    sample += "\"gyro_x\":" + String(gx) + ",";
    sample += "\"gyro_y\":" + String(gy) + ",";
    sample += "\"gyro_z\":" + String(gz) + ",";
    sample += "\"label\":\"falling\"";
    sample += "}";

    dataBuffer += sample;
    bufferCount++;

    if (bufferCount < batchSize) {
      dataBuffer += ",";  // Add comma only between samples
    } else {
      dataBuffer += "]";  // Close JSON array

      if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(serverUrl);
        http.addHeader("Content-Type", "application/json");

        int httpCode = http.POST(dataBuffer);
        Serial.print("HTTP Response Code: ");
        Serial.println(httpCode);
        http.end();
        Serial.print("Sent batch of ");
        Serial.print(bufferCount);
        Serial.println(" samples");
      }

      // Reset for next batch
      dataBuffer = "[";
      bufferCount = 0;
    }
  }
}
