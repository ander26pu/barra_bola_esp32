import serial
import matplotlib.pyplot as plt
from collections import deque
import time
import sys

# ——— Configuración serial ———
SERIAL_PORT = 'COM4'   # ajústalo si no es COM4
BAUD_RATE   = 115200

try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
except serial.SerialException as e:
    print(f"Error al abrir el puerto serial: {e}")
    sys.exit(1)

# ——— Parámetros de la gráfica ———
max_len     = 200
roll_data   = deque(maxlen=max_len)  # Deque vacío
pitch_data  = deque(maxlen=max_len)  # Deque vacío

# ——— Función de parseo ———
def parse_line(line):
    line = line.replace("°","")
    if "Roll" in line and "Pitch" in line:
        try:
            r_str, p_str = line.split(',')
            r = float(r_str.split(':')[1].strip())
            p = float(p_str.split(':')[1].strip())
            return r, p
        except Exception as e:
            print(f"Error de parseo: {e}")
    return None

# ——— Inicializar Matplotlib en modo interactivo ———
plt.ion()
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(8, 6), sharex=True)

# Configurar ejes
ax1.set_ylim(-90, 90)
ax1.set_ylabel("Roll (°)")
ax1.grid(True)
ax1.set_title("Ángulos de Orientación en Tiempo Real")

ax2.set_ylim(-90, 90)
ax2.set_ylabel("Pitch (°)")
ax2.set_xlabel("Muestras")
ax2.grid(True)

# Crear líneas vacías
roll_line, = ax1.plot([], [], 'b-', linewidth=1.5)
pitch_line, = ax2.plot([], [], 'r-', linewidth=1.5)

# Añadir leyendas
ax1.legend(['Roll'], loc='upper right')
ax2.legend(['Pitch'], loc='upper right')

# ——— Bucle principal ———
sample_count = 0
last_update_time = time.time()
update_interval = 0.03  # Actualizar cada 30 ms (~33 FPS)

try:
    while True:
        # Leer todas las líneas disponibles para evitar acumulación
        while ser.in_waiting:
            try:
                raw = ser.readline().decode('utf-8', errors='ignore').strip()
                res = parse_line(raw)
                if res:
                    r, p = res
                    roll_data.append(r)
                    pitch_data.append(p)
                    sample_count += 1
            except Exception as e:
                print(f"Error de lectura: {e}")
        
        # Actualizar gráficos solo si tenemos datos y ha pasado el intervalo mínimo
        current_time = time.time()
        if roll_data and pitch_data and (current_time - last_update_time > update_interval):
            # Crear índices para el eje X basados en el número de muestras
            x_vals = list(range(max(0, sample_count - len(roll_data)), sample_count))
            
            # Actualizar datos de las líneas
            roll_line.set_data(x_vals, roll_data)
            pitch_line.set_data(x_vals, pitch_data)
            
            # Ajustar límites del eje X para desplazamiento automático
            ax1.set_xlim(x_vals[0], x_vals[-1] + 1)
            ax2.set_xlim(x_vals[0], x_vals[-1] + 1)
            
            # Redibujar
            fig.canvas.draw_idle()
            fig.canvas.flush_events()
            
            last_update_time = current_time
        
        # Pequeña pausa para no saturar la CPU
        time.sleep(0.001)

except KeyboardInterrupt:
    print("\nInterrupción por usuario. Cerrando...")
except Exception as e:
    print(f"Error inesperado: {e}")
finally:
    ser.close()
    plt.ioff()
    plt.show()