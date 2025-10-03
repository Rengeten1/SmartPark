#include <Arduino.h>

// Sensor pins
#define TRIG_PIN1 3
#define ECHO_PIN1 2
#define TRIG_PIN2 9
#define ECHO_PIN2 8

// -------------------- Parameters --------------------
#define CAR_THRESHOLD 20   // cm: distance to detect a car

// -------------------- Global Variables --------------------
int carCount = 0;
bool waitingForSensor2 = false;

// Track previous state of Sensor 2 for edge detection
bool sensor2PreviouslyHigh = false;

// -------------------- Functions --------------------
long getDistanceCM(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 40000); // 40 ms timeout
  if (duration == 0) return -1;
  long distance = duration * 0.034 / 2;
  return distance > 400 ? 400 : distance;
}

// -------------------- Setup --------------------
void setup() {
  Serial.begin(9600);

  pinMode(TRIG_PIN1, OUTPUT);
  pinMode(ECHO_PIN1, INPUT);
  pinMode(TRIG_PIN2, OUTPUT);
  pinMode(ECHO_PIN2, INPUT);

  Serial.println("Traffic counter started");
}

// -------------------- Main Loop --------------------
void loop() {
  long dist1 = getDistanceCM(TRIG_PIN1, ECHO_PIN1);
  delay(50);
  long dist2 = getDistanceCM(TRIG_PIN2, ECHO_PIN2);
  delay(50);

  // If not waiting for sensor2, check sensor1
  if (!waitingForSensor2) {
    if (dist1 != -1 && dist1 < CAR_THRESHOLD) {
      waitingForSensor2 = true;
      Serial.println("Sensor 1 detected car, waiting for sensor 2...");
    }
  } 
  // If waiting for sensor2
  else {
    bool sensor2High = (dist2 != -1 && dist2 < CAR_THRESHOLD);

    // Only increment when sensor2 transitions from no car → car detected
    if (sensor2High && !sensor2PreviouslyHigh) {
      carCount++;
      Serial.print("Car passed! Total: ");
      Serial.println(carCount);

      // Reset for next car
      waitingForSensor2 = false;
    }

    sensor2PreviouslyHigh = sensor2High;
  }
}
