#include <Arduino.h>
#include <Wire.h>
#include <BMI160Gen.h>
#include <ESP32Servo.h>
#include <math.h>

// —————— Configuración del Servo ——————
const int servoPin     = 18;
Servo    myServo;
const int minPulseUs   = 500;
const int maxPulseUs   = 2500;
float     frequencyHz  = 0.5;      // Hz de la oscilación
int       angleMin     = 135;
int       angleMax     = 170;
const unsigned long intervalMs = 5;    // actualización del servo (~200 Hz)
unsigned long prevServoMs = 0;
unsigned long servoStartMs = 0;

// —————— Configuración del BMI160 ——————
const int i2c_addr   = 0x68;
const int sda_pin    = 21;
const int scl_pin    = 22;
const float ACCEL_SENS = 16384.0; // ±2g
const float GYRO_SENS  = 131.0;   // ±250°/s
const float alpha      = 0.98;    // filtro complementario
float roll = 0.0, pitch = 0.0;
unsigned long lastSensorMs = 0;

// —————— Setup ——————
void setup() {
  // Serial
  Serial.begin(115200);
  while (!Serial) delay(10);

  // BMI160
  Wire.begin(sda_pin, scl_pin);
  if (!BMI160.begin(BMI160GenClass::I2C_MODE, i2c_addr)) {
    Serial.println("¡Fallo al inicializar el BMI160!");
    while (1);
  }
  Serial.println("BMI160 inicializado correctamente.");

  // Servo
  myServo.setPeriodHertz(50);
  myServo.attach(servoPin, minPulseUs, maxPulseUs);
  float midAngle = (angleMin + angleMax) * 0.5;
  writeServo(midAngle);
  delay(500);

  // Tiempos iniciales
  lastSensorMs = millis();
  prevServoMs  = millis();
  servoStartMs = millis();

  Serial.println("[Setup] Sistema combinado listo");
  Serial.println("Comandos: FREQ <Hz>, RANGE <min> <max>");
}

// —————— Loop principal ——————
void loop() {
  unsigned long now = millis();

  // —— 1) Lectura y cálculo del ángulo (≈100 Hz) ——
  if (now - lastSensorMs >= 10) {
    lastSensorMs = now;
    readAndComputeIMU(now);
  }

  // —— 2) Actualización del servo (≈200 Hz) ——
  if (now - prevServoMs >= intervalMs) {
    prevServoMs = now;
    processSerialCommands(now);
    updateServo(now);
  }
}

// —————— Funciones auxiliares ——————

void readAndComputeIMU(unsigned long now) {
  int ax_raw, ay_raw, az_raw;
  int gx_raw, gy_raw, gz_raw;
  BMI160.readAccelerometer(ax_raw, ay_raw, az_raw);
  BMI160.readGyro(gx_raw, gy_raw, gz_raw);

  // Escalas
  float ax = ax_raw / ACCEL_SENS;
  float ay = ay_raw / ACCEL_SENS;
  float az = az_raw / ACCEL_SENS;
  float gx = gx_raw / GYRO_SENS;
  float gy = gy_raw / GYRO_SENS;

  // dt
  static unsigned long lastT = now;
  float dt = (now - lastT) * 0.001;
  lastT = now;

  // Ángulos del acelerómetro
  float rollAcc  = atan2(ay, az) * 180.0 / PI;
  float pitchAcc = atan2(-ax, sqrt(ay*ay + az*az)) * 180.0 / PI;

  // Integración giroscopio
  roll  += gx * dt;
  pitch += gy * dt;

  // Complementario
  roll  = alpha * roll  + (1 - alpha) * rollAcc;
  pitch = alpha * pitch + (1 - alpha) * pitchAcc;

  // Mostrar
  Serial.print("Roll: ");
  Serial.print(roll, 2);
  Serial.print("°, Pitch: ");
  Serial.print(pitch, 2);
  Serial.println("°");
}

void processSerialCommands(unsigned long now) {
  if (!Serial.available()) return;
  String cmd = Serial.readStringUntil('\n');
  cmd.trim();
  if (cmd.startsWith("FREQ")) {
    float f = cmd.substring(4).toFloat();
    if (f > 0) {
      frequencyHz = f;
      Serial.printf("Freq=%.2fHz\n", frequencyHz);
    } else {
      Serial.println("Invalid FREQ");
    }
    servoStartMs = now;
  }
  else if (cmd.startsWith("RANGE")) {
    int first = cmd.indexOf(' ');
    int second = cmd.indexOf(' ', first+1);
    int mn = cmd.substring(first+1, second).toInt();
    int mx = cmd.substring(second+1).toInt();
    if (mn >= 0 && mx <= 180 && mn < mx) {
      angleMin = mn; angleMax = mx;
      Serial.printf("Range=%d-%d\n", angleMin, angleMax);
    } else {
      Serial.println("Invalid RANGE");
    }
    servoStartMs = now;
  }
}

void updateServo(unsigned long now) {
  float t     = (now - servoStartMs) * 0.001;
  float omega = 2 * PI * frequencyHz;
  float amp   = (angleMax - angleMin) * 0.5;
  float mid   = (angleMax + angleMin) * 0.5;
  float angle = mid + amp * sin(omega * t);
  writeServo(angle);
}

void writeServo(float angle) {
  float pw = minPulseUs + (angle / 180.0) * (maxPulseUs - minPulseUs);
  myServo.writeMicroseconds(int(pw + 0.5));
}

