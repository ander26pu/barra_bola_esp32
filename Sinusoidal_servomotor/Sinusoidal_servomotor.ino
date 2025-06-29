// ESP32 + DS3218 Servo Smooth Continuous Sinusoidal Control
// Uses external 6V supply (5-6.5V recommended)
// Connections:
//   - Servo VCC (red) to external 6V supply
//   - Servo GND (black/brown) to common GND
//   - Servo signal (yellow/orange) to ESP32 GPIO

#include <Arduino.h>
#include <ESP32Servo.h>

const int servoPin = 18;
Servo myServo;

// Sinusoidal parameters
float frequencyHz = 0.5;        // Hz
int angleMin = 155;
int angleMax = 165;

// Pulse width bounds
const int minPulseUs = 500;
const int maxPulseUs = 2500;

// Timing
unsigned long prevMillis = 0;
const unsigned long intervalMs = 5;  // 5ms ~200Hz update for smoothness
unsigned long startTime;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  Serial.println("[Setup] Starting Smooth Sinusoidal Control");
  Serial.println("Commands: FREQ <Hz>, RANGE <min> <max>");

  myServo.setPeriodHertz(50);
  myServo.attach(servoPin, minPulseUs, maxPulseUs);

  // center at midpoint
  float mid = (angleMin + angleMax) * 0.5;
  writeAngle(mid);
  delay(500);
  startTime = millis();
}

void loop() {
  unsigned long now = millis();
  if (now - prevMillis < intervalMs) return;
  prevMillis = now;

  // process serial
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n'); cmd.trim();
    if (cmd.startsWith("FREQ")) {
      float f = cmd.substring(4).toFloat();
      if (f > 0) { frequencyHz = f; Serial.printf("Freq=%.2fHz\n", frequencyHz);} 
      else Serial.println("Invalid FREQ");
      startTime = millis();
    } else if (cmd.startsWith("RANGE")) {
      int first = cmd.indexOf(' ');
      int second = cmd.indexOf(' ', first+1);
      int mn = cmd.substring(first+1, second).toInt();
      int mx = cmd.substring(second+1).toInt();
      if (mn>=0 && mx<=180 && mn<mx) { angleMin=mn; angleMax=mx; Serial.printf("Range=%d-%d\n", mn, mx); }
      else Serial.println("Invalid RANGE");
      startTime = millis();
    }
  }

  // compute float angle
  float t = (now - startTime)*0.001;
  float omega = 2*PI*frequencyHz;
  float amp = (angleMax - angleMin)*0.5;
  float mid = (angleMax + angleMin)*0.5;
  float angleF = mid + amp * sin(omega * t);

  writeAngle(angleF);
}

void writeAngle(float angleF) {
  // convert angleF [0-180] to pulse width
  float pw = minPulseUs + (angleF/180.0)*(maxPulseUs - minPulseUs);
  int pulse = (int)(pw + 0.5);
  myServo.writeMicroseconds(pulse);
}

// Usage: Serial 115200
// FREQ <Hz>
// RANGE <min> <max>

// For ultra-smooth, intervalMs can be lowered further.
