// ESP32 + DS3218 Servo Control with Serial Angle & Speed Commands
// Power DS3218 with external 6V supply (5-6.5V recommended)
// Connections:
//   - Servo VCC (red) to external 6V supply
//   - Servo GND (black/brown) to common GND (tie ESP32 GND and external supply GND)
//   - Servo signal (yellow/orange) to ESP32 GPIO

#include <Arduino.h>
#include <ESP32Servo.h>  // Install via Library Manager

const int servoPin = 18;        // PWM-capable GPIO on ESP32
Servo myServo;

// Movement parameters
int targetAngle = 160;           // Default center
int currentAngle = 160;
int moveDelayMs = 50;           // Delay between 1° steps: lower = faster, default slowest

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  Serial.println("[Setup] ESP32Servo Initialized. Type 'ANGLE <0-180>' or 'SPEED <delay_ms>'");

  // Configure servo
  myServo.setPeriodHertz(50);               // Standard servo frequency
  myServo.attach(servoPin, 500, 2500);      // minPulse 500us, maxPulse 2500us

  // Start at default
  myServo.write(currentAngle);
}

void loop() {
  // Handle serial commands if available
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd.startsWith("ANGLE")) {
      int angle = cmd.substring(5).toInt();
      if (angle >= 0 && angle <= 180) {
        targetAngle = angle;
        Serial.printf("[Command] New target angle: %d°\n", targetAngle);
      } else {
        Serial.println("[Error] ANGLE must be 0-180");
      }
    } else if (cmd.startsWith("SPEED")) {
      int d = cmd.substring(5).toInt();
      if (d >= 5 && d <= 1000) {
        moveDelayMs = d;
        Serial.printf("[Command] New speed delay: %d ms per degree\n", moveDelayMs);
      } else {
        Serial.println("[Error] SPEED must be 5-1000 ms");
      }
    } else {
      Serial.println("[Error] Invalid command. Use 'ANGLE <0-180>' or 'SPEED <5-1000>'");
    }
  }

  // Gradually move to targetAngle
  if (currentAngle < targetAngle) {
    currentAngle++;
    myServo.write(currentAngle);
    delay(moveDelayMs);
  } else if (currentAngle > targetAngle) {
    currentAngle--;
    myServo.write(currentAngle);
    delay(moveDelayMs);
  }
}

// Usage examples via Serial Monitor (baud 115200):
//   ANGLE 45    -> move servo to 45°
//   SPEED 50    -> set delay to 50ms per degree (faster)
// Default SPEED = 30 ms per degree (slowest smooth movement)
// Limits: ANGLE 0-180, SPEED 5-1000

// Tips:
// - Ensure common ground and decoupling cap (100µF) near servo power.
// - Verify power supply can deliver sufficient current (>1A peak).
