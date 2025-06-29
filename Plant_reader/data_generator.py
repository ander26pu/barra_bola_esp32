#!/usr/bin/env python3
import serial
import time
import csv
import sys
import argparse

def main(port='COM4', baud=115200, timeout=5, output_file='datos_planta.csv',
         duration=10.0):
    """
    Lee t, u, v desde el ESP32 y guarda durante 'duration' segundos.
    """
    try:
        ser = serial.Serial(port, baud, timeout=timeout)
    except Exception as e:
        print(f"Error abriendo {port}: {e}")
        sys.exit(1)

    # Dar tiempo a que el ESP32 arranque
    time.sleep(2)
    ser.write(b'START\n')

    datos = []
    start_time = time.time()
    print(f"Capturando datos durante {duration:.1f} segundos. Presiona Ctrl+C para abortar antes.")

    try:
        while True:
            line = ser.readline().decode('utf-8', errors='ignore').strip()
            if not line:
                continue

            # Ignorar cabecera
            if line.startswith('t['):
                continue

            parts = line.split(',')
            # Esperamos exactamente 3 valores: t, u, v
            if len(parts) != 3:
                continue

            try:
                t = float(parts[0])
                u = float(parts[1])
                v = float(parts[2])
            except ValueError:
                continue

            datos.append([t, u, v])
            print(f"{t:.3f}, {u:.2f}, {v:.2f}")

            # ¿Hemos capturado suficiente tiempo?
            if (time.time() - start_time) >= duration:
                print(f"\nDuración de {duration:.1f}s alcanzada.")
                break

    except KeyboardInterrupt:
        print("\nInterrupción recibida, terminando captura.")
    finally:
        ser.close()

    # Guardar CSV
    with open(output_file, 'w', newline='') as f:
        writer = csv.writer(f)
        writer.writerow(['t','u','v'])
        writer.writerows(datos)

    print(f"Guardadas {len(datos)} muestras en '{output_file}'")

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Lectura t,u,v desde ESP32 con duración limitada")
    parser.add_argument('--port',     default='COM4',         help='Puerto serial (p.ej. COM4 o /dev/ttyUSB0)')
    parser.add_argument('--baud',     type=int, default=115200, help='Baudrate')
    parser.add_argument('--output',   default='datos_planta.csv', help='Archivo CSV de salida')
    parser.add_argument('--duration', type=float, default=10.0,  help='Tiempo de captura en segundos')
    args = parser.parse_args()

    main(
        port=args.port,
        baud=args.baud,
        timeout=5,
        output_file=args.output,
        duration=args.duration
    )
