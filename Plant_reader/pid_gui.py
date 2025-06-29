import threading
import time
import customtkinter as ctk
import serial
import serial.tools.list_ports
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from collections import deque

# Apariencia
ctk.set_appearance_mode("System")
ctk.set_default_color_theme("blue")

class SerialInterface:
    def __init__(self, port, baud=115200):
        self.queue = deque()
        try:
            self.ser = serial.Serial(port, baud, timeout=0.1)
        except:
            self.ser = None
        self.lock = threading.Lock()
        self.running = True
        if self.ser:
            threading.Thread(target=self._read_loop, daemon=True).start()

    def _read_loop(self):
        while self.running:
            line = self.ser.readline().decode(errors='ignore').strip()
            if line:
                with self.lock:
                    self.queue.append(line)
        self.ser.close()

    def write(self, cmd):
        if self.ser:
            self.ser.write((cmd + '\n').encode())

    def get_all(self):
        lines = []
        with self.lock:
            while self.queue:
                lines.append(self.queue.popleft())
        return lines

    def close(self):
        self.running = False
        if self.ser:
            self.ser.close()

class App(ctk.CTk):
    def __init__(self):
        super().__init__()
        self.title("Control Bola-Barra GUI")
        self.geometry("1000x600")

        # Datos circular para rendimiento
        self.times = deque(maxlen=500)
        self.distances = deque(maxlen=500)
        self.setpoints = deque(maxlen=500)
        self.start_time = time.time()
        self.iface = None

        self._build_ui()
        self.after(20, self._read_serial)   # lectura más frecuente
        self.after(50, self._update_plot)   # actualización más frecuente

    def _build_ui(self):
        # Frames
        left = ctk.CTkFrame(self)
        left.pack(side="left", fill="both", expand=True)
        right = ctk.CTkScrollableFrame(self, width=300)
        right.pack(side="right", fill="y")

        # Gráfica
        self.fig, self.ax = plt.subplots()
        self.line_dist, = self.ax.plot([], [], label="Distancia")
        self.line_set, = self.ax.plot([], [], label="Setpoint")
        self.ax.legend(); self.ax.set_xlabel("Tiempo (s)"); self.ax.set_ylabel("mm")
        self.canvas = FigureCanvasTkAgg(self.fig, left)
        self.canvas.get_tk_widget().pack(fill="both", expand=True)

        # Controles básicos
        params = ["SET","KP","KI","KD","AMP","CYCLES","ERR MIN","ERR MAX"]
        row = 0
        self.entries = {}
        for p in params:
            lbl = ctk.CTkLabel(right, text=p)
            lbl.grid(row=row, column=0, pady=2, sticky="w")
            ent = ctk.CTkEntry(right, width=80)
            ent.grid(row=row, column=1, pady=2)
            default = {"SET":"100","KP":"0.139","KI":"0.116","KD":"0.042",
                       "AMP":"20","CYCLES":"6","ERR MIN":"-3","ERR MAX":"3"}[p]
            ent.insert(0, default)
            self.entries[p] = ent
            row += 1
        # Botones y switch
        btn_err = ctk.CTkButton(right, text="Apply ERROR", command=self._apply_error)
        btn_err.grid(row=row, columnspan=2, pady=5); row+=1
        self.tune_var = ctk.BooleanVar(value=False)
        sw = ctk.CTkSwitch(right, text="Autotune", variable=self.tune_var, command=self._toggle_tune)
        sw.grid(row=row, columnspan=2, pady=5); row+=1
        ctk.CTkLabel(right, text="Puerto Serie:").grid(row=row, column=0, pady=5)
        self.port_box = ctk.CTkOptionMenu(right, values=self._list_ports())
        self.port_box.grid(row=row, column=1, pady=5); row+=1
        btn_conn = ctk.CTkButton(right, text="Conectar", command=self._connect_serial)
        btn_conn.grid(row=row, columnspan=2, pady=10)

    def _list_ports(self):
        return [p.device for p in serial.tools.list_ports.comports()]

    def _connect_serial(self):
        port = self.port_box.get()
        if self.iface: self.iface.close()
        self.iface = SerialInterface(port)
        # mandar valor set actual
        self._send(f"SET {self.entries['SET'].get()}")

    def _send(self, cmd):
        if self.iface: self.iface.write(cmd)

    def _apply_error(self):
        self._send(f"ERROR {self.entries['ERR MIN'].get()} {self.entries['ERR MAX'].get()}")

    def _toggle_tune(self):
        cmd = "TUNE ON" if self.tune_var.get() else "TUNE OFF"
        self._send(cmd)

    def _read_serial(self):
        if self.iface:
            for line in self.iface.get_all():
                if line.startswith("Dist:"):
                    dist = float(line.split()[0].split(':')[1])
                    sp = float(self.entries['SET'].get())
                    t = time.time() - self.start_time
                    self.times.append(t); self.distances.append(dist); self.setpoints.append(sp)
        self.after(20, self._read_serial)

    def _update_plot(self):
        self.line_dist.set_data(self.times, self.distances)
        self.line_set.set_data(self.times, self.setpoints)
        self.ax.relim(); self.ax.autoscale_view()
        self.canvas.draw_idle()
        self.after(50, self._update_plot)

if __name__ == "__main__":
    App().mainloop()
