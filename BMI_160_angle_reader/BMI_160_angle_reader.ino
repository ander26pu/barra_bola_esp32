#include <BMI160Gen.h>
#include <Wire.h>
#include <math.h>

// I2C
const int i2c_addr = 0x68;
const int sda_pin = 21;
const int scl_pin = 22;

// Escalas
const float ACCEL_SENS = 16384.0; // ±2g → 1g = 16384 LSB
const float GYRO_SENS = 131.0;    // ±250°/s → 1°/s = 131 LSB

// Ángulos calculados (inicialmente 0)
float roll = 0.0;
float pitch = 0.0;

// Filtro complementario
const float alpha = 0.98; // peso del giroscopio

unsigned long lastTime = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Wire.begin(sda_pin, scl_pin);

  if (!BMI160.begin(BMI160GenClass::I2C_MODE, i2c_addr)) {
    Serial.println("¡Fallo al inicializar el BMI160!");
    while (1);
  }

  Serial.println("BMI160 inicializado correctamente.");
  lastTime = millis();
}

void loop() {
  int ax_raw, ay_raw, az_raw;
  int gx_raw, gy_raw, gz_raw;

  BMI160.readAccelerometer(ax_raw, ay_raw, az_raw);
  BMI160.readGyro(gx_raw, gy_raw, gz_raw);

  float ax = ax_raw / ACCEL_SENS;
  float ay = ay_raw / ACCEL_SENS;
  float az = az_raw / ACCEL_SENS;

  float gx = gx_raw / GYRO_SENS; // °/s
  float gy = gy_raw / GYRO_SENS;

  unsigned long now = millis();
  float dt = (now - lastTime) / 1000.0; // en segundos
  lastTime = now;

  // --- Estimación por acelerómetro ---
  float rollAcc  = atan2(ay, az) * 180.0 / PI;
  float pitchAcc = atan2(-ax, sqrt(ay * ay + az * az)) * 180.0 / PI;

  // --- Integración del giroscopio ---
  roll += gx * dt;
  pitch += gy * dt;

  // --- Filtro complementario ---
  roll  = alpha * roll  + (1 - alpha) * rollAcc;
  pitch = alpha * pitch + (1 - alpha) * pitchAcc;

  // --- Mostrar resultados ---
  Serial.print("Roll: ");
  Serial.print(roll, 2);
  Serial.print("°, Pitch: ");
  Serial.print(pitch, 2);
  Serial.println("°");

  delay(10); // ~100 Hz
}
