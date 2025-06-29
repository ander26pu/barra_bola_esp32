% calcular_entrada_seno.m
% Dado G(s)=2.6531-0.0265 s / (1*s+3.9537)
% Calcula y grafica la señal de entrada requerida para una salida seno de ±2° a 0.5 Hz

clc; clear; close all;

% Definir la TF continua G(s)
num = [-0.0265 2.6531];   % coeficientes de numerador
den = [1 3.9537];         % coeficientes de denominador
G   = tf(num, den);

disp('Transferencia G(s):'); disp(G);

% Parámetros de la señal de salida deseada
a_out = 2;               % amplitud en grados
f     = 0.5;             % frecuencia en Hz
dt    = 0.001;           % intervalo de muestreo (1 kHz)
t    = 0:dt:2;           % vector tiempo para dos ciclos

% Generar señal de salida deseada
y_des = a_out * sin(2*pi*f*t);

% Calcular respuesta de G en jw
w    = 2*pi*f;
H    = evalfr(G, 1i*w);
magH = abs(H);
angH = angle(H);

% Entrada requerida de amplitud y fase ajustada
a_in  = a_out / magH;
phi_in = -angH;
u_req = a_in * sin(2*pi*f*t + phi_in);

% Graficar ambas señales
g = figure;
plot(t, y_des, 'r--', 'LineWidth',1.5); hold on;
plot(t, u_req, 'b-', 'LineWidth',1.5);
xlabel('Tiempo (s)'); ylabel('Ángulo (°)');
legend('Salida deseada y_{des}','Entrada requerida u_{req}');
title('Entrada para salida seno ±2° @0.5Hz');
grid on;

% Opcional: dibujar segundo gráfico solo de la entrada
figure;
plot(t, u_req, 'b-', 'LineWidth',1.5);
xlabel('Tiempo (s)'); ylabel('u_{req} (°)');
title('Señal de entrada requerida'); grid on;
