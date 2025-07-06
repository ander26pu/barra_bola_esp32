# Sistema Barra-Bola con ESP32

Este proyecto implementa un sistema barra-bola usando un microcontrolador ESP32, un servomotor DS3218, y sensores como el VL53L0X (distancia) y el BMI160 (inclinación). Se documentan todas las etapas del desarrollo incluyendo diseño, pruebas, identificación de planta, controladores PID y Fuzzy, y evaluación de desempeño.

---

## 🔧 1. Diseño Mecánico

* [✅] Prototipo inicial funcional, aunque inestable a alta velocidad.
* [✅] Soporte central añadido con sensor **BMI160** para medir inclinación real.
* [✅] Soporte adicional para posible segundo sensor.

**Observaciones:**

* La flexibilidad de la barra genera vibraciones.
* Tener el ángulo real mejora la precisión del modelo.

---

## ⚡ 2. Diseño Electrónico

### 2.1 Sensor de Distancia (VL53L0X v2)

* [ ] Verificación de exactitud y precisión.
* [ ] Evaluación del uso del pin **GPIO1** para mayor velocidad.
* [ ] Comparación de librerías disponibles.
* [ ] Conversión de unidades a **cm** y definición del **cero**.
* [ ] Evaluar interrupciones usando **GPIO0**.

### 2.2 Servomotor (DS3218)

* [ ] Evaluar control con PWM vs. librería.
* [ ] Suavizar movimientos para evitar vibraciones.
* [ ] Definir rangos:

  * Rango de operación.
  * Rango de seguridad.
  * Velocidad angular máxima.
* [ ] Identificar ángulo horizontal de referencia.
* [ ] Implementación por interrupciones.

### 2.3 Sensor de Inclinación (BMI160)

* [ ] Pruebas de lectura de ángulo.
* [ ] Determinar sensibilidad al movimiento de la bola.
* [ ] Determinar ángulo mínimo de movimiento.

### 2.4 Integración de Sensores y Actuador

* [ ] Generar señal seno en servo y verificar lectura de sensores.
* [ ] Estimar frecuencia máxima sin colisión.
* [ ] Verificar sincronización.

### 2.5 Visualización y Adquisición de Datos

* [ ] Transmisión de datos por **PySerial**.
* [ ] Visualización en tiempo real.
* [ ] Grabación en `.csv` para análisis.

---

## 🧐 3. Identificación de Planta

* [ ] Calcular tiempo de muestreo desde CSV.
* [ ] Identificación de sistema en tiempo discreto (plano z).
* [ ] Considerar retardos de sensor.

---

## 🚪 4. Diseño de Control PID

* [ ] Diseño en MATLAB.
* [ ] Implementación discreta.
* [ ] Prueba de estabilidad y desempeño.

---

## 🌐 5. Diseño de Control Fuzzy

* [ ] Diseño de controlador fuzzy en discreto.
* [ ] Implementación en **ESP32 con Arduino IDE**.
* [ ] Evaluación frente a PID.

---

## 📊 6. Medición de Desempeño

* [ ] Tiempo de establecimiento.
* [ ] Sobrepaso y error en estado estacionario.
* [ ] Comparación PID vs. Fuzzy.

---

## 🗂️ Registro de Cambios

| Fecha      | Cambio realizado                           | Notas técnicas                |
| ---------- | ------------------------------------------ | ----------------------------- |
| 2025-07-06 | Soporte central añadido + BMI160 instalado | Mejora estabilidad angular    |
| 2025-07-07 | Test inicial de sensor VL53L0X             | Buen rango, necesita filtrado |
| 2025-07-08 | Implementado PWM gradual para servo        | Reducción de vibración        |
| ...        | ...                                        | ...                           |

---

## 📅 Estado del Proyecto

**En desarrollo activo**. Se recomienda revisar el registro de cambios para ver el progreso diario.

---

> ✨ Este README es el documento principal del repositorio y se actualizará continuamente conforme avance el proyecto. Si estás leyendo esto, estás viendo la historia de cómo se construyó paso a paso este sistema barra-bola.
