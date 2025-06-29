#include <Arduino.h>
#include <Wire.h>
#include <BMI160Gen.h>
#include <ESP32Servo.h>
#include <math.h>
#include <Ticker.h>

// —————— Configuración del Servo ——————
const int     servoPin     = 18;
Servo         myServo;
const int     minPulseUs   = 500;
const int     maxPulseUs   = 2500;
float         frequencyHz  = 0.5;      // Hz de la oscilación
int           angleMin     = 144;
int           angleMax     = 170;

// —————— Variables de temporización ——————
Ticker        sampler;
unsigned long startTime    = 0;
const uint32_t sampleIntMs = 20;       // Intervalo de muestreo en ms (~50 Hz)

// —————— Variables para cálculo de ángulo y sensor ——————
const float   ACCEL_SENS   = 16384.0;
const float   GYRO_SENS    = 131.0;
const float   alpha        = 0.98;
float         roll         = 0.0;
unsigned long lastSensorMs = 0;

// —————— Prototipos ——————
void onSample();
void writeServo(float angle);
void readAndComputeIMU(unsigned long now);
void processSerialCommands(unsigned long now);

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  // Inicializar BMI160
  Wire.begin(21, 22);
  if (!BMI160.begin(BMI160GenClass::I2C_MODE, 0x68)) {
    Serial.println("¡Fallo al inicializar el BMI160!");
    while (1);
  }
  Serial.println("BMI160 listo.");

  // Inicializar Servo
  myServo.setPeriodHertz(50);
  myServo.attach(servoPin, minPulseUs, maxPulseUs);
  float mid = (angleMin + angleMax) * 0.5;
  writeServo(mid);
  delay(500);

  // Mensaje y configuración del muestreo
  Serial.println("Enviar 'START' para comenzar muestreo.");
}

void loop() {
  // Arranca el sampler cuando reciba START
  if (Serial.find("START\n")) {
    Serial.println("t[s],u[deg],v_roll[deg]");
    startTime     = millis();
    lastSensorMs  = startTime;
    sampler.attach_ms(sampleIntMs, onSample);
  }
}

// Esta función se llama periódicamente por el Ticker
void onSample() {
  unsigned long now = millis();
  float t = (now - startTime) * 0.001f;    // tiempo en segundos

  // --- 1) Señal de entrada u: ángulo sinusoidal ---
  float omega = 2 * PI * frequencyHz;
  float amp   = (angleMax - angleMin) * 0.5;
  float mid   = (angleMax + angleMin) * 0.5;
  float u     = mid + amp * sin(omega * t);
  writeServo(u);

  // --- 2) Lectura y cálculo IMU para roll ---
  readAndComputeIMU(now);
  float v = roll;

  // --- 3) Envío Serial: tiempo, u, v ---
  Serial.printf("%.3f,%.2f,%.2f\n", t, u, v);
}

// Convierte un ángulo [0–180] a microsegundos y escribe al servo
void writeServo(float angle) {
  float pw = minPulseUs + (angle / 180.0f) * (maxPulseUs - minPulseUs);
  myServo.writeMicroseconds(int(pw + 0.5f));
}

// Lee acelerómetro+giroscopio, aplica filtro complementario y actualiza roll
void readAndComputeIMU(unsigned long now) {
  static unsigned long lastT = 0;
  if (lastT == 0) lastT = now;

  int ax_raw, ay_raw, az_raw;
  int gx_raw, gy_raw, gz_raw;
  BMI160.readAccelerometer(ax_raw, ay_raw, az_raw);
  BMI160.readGyro(gx_raw, gy_raw, gz_raw);

  float ax = ax_raw / ACCEL_SENS;
  float ay = ay_raw / ACCEL_SENS;
  float az = az_raw / ACCEL_SENS;
  float gx = gx_raw / GYRO_SENS;
  float gy = gy_raw / GYRO_SENS;

  float dt = (now - lastT) * 0.001f;
  lastT = now;

  float rollAcc  = atan2(ay, az) * 180.0f / PI;
  roll += gx * dt;
  roll = alpha * roll + (1 - alpha) * rollAcc;
}
