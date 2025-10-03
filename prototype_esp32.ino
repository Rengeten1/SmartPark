#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

// ====== WiFi Config ======
const char* ssid = "ssid";
const char* password = "passwored";

// ====== Server Config ======
// Change this to your server URL or endpoint
String serverName = "http://your-server.com/api/";

// ====== Sensor Pins ======
#define TRIG_PIN1 5
#define ECHO_PIN1 18
#define TRIG_PIN2 19
#define ECHO_PIN2 21

// ====== Car Detection Params ======
#define CAR_THRESHOLD 20      // cm
#define MAX_WAIT 10000        // ms

// ====== Variables ======
int carCount = 0;
bool sensor1Triggered = false;
unsigned long sensor1Time = 0;

long getDistanceCM(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 40000);
  if (duration == 0) return -1; 
  long distance = duration * 0.034 / 2;
  return distance > 400 ? 400 : distance;
}

void sendCarCount() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    http.begin(serverName);  
    http.addHeader("Content-Type", "application/json");

    // Prepare JSON data
    String payload = "{\"car_count\": " + String(carCount) + "}";

    int httpResponseCode = http.POST(payload);

    if (httpResponseCode > 0) {
      Serial.print("POST Response code: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Error sending POST: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi not connected cannot send data");
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN1, OUTPUT);
  pinMode(ECHO_PIN1, INPUT);

  pinMode(TRIG_PIN2, OUTPUT);
  pinMode(ECHO_PIN2, INPUT);

  // WiFi connect
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
}

void loop() {
  long dist1 = getDistanceCM(TRIG_PIN1, ECHO_PIN1);
  delay(50);
  long dist2 = getDistanceCM(TRIG_PIN2, ECHO_PIN2);
  delay(50);

  unsigned long currentMillis = millis();

  // Sensor 1 detects car
  if (!sensor1Triggered && dist1 != -1 && dist1 < CAR_THRESHOLD) {
    sensor1Triggered = true;
    sensor1Time = currentMillis;
    Serial.println("Sensor 1 detected car, timer started");
  }

  // Sensor 2 detects car within 10s
  if (sensor1Triggered && dist2 != -1 && dist2 < CAR_THRESHOLD) {
    if (currentMillis - sensor1Time <= MAX_WAIT) {
      carCount++;
      Serial.print("Car passed! Total: ");
      Serial.println(carCount);

      sendCarCount();  // 🚀 Send data to server

      sensor1Triggered = false;
    }
  }

  // Timer expired
  if (sensor1Triggered && currentMillis - sensor1Time > MAX_WAIT) {
    sensor1Triggered = false;
    Serial.println("Timer expired, no car detected on sensor 2");
  }
}
