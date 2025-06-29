% identificador_planta.m
% Identificación automática de la planta bola–barra (ángulos)
% Basado en excitación OE con System Identification Toolbox

clc; clear; close all;

% --- Parámetros de entrada ---
csvFile = 'datos_planta.csv';
N       = 20;   % ventana de media móvil para filtrado
nb      = 1;    % número de polos del modelo OE a estimar
nf      = 1;    % número de ceros del modelo OE a estimar
nk      = 1;    % delay en muestras

% --- Leer y preprocesar datos ---
T        = readtable(csvFile);
time     = T.t;         % tiempo (s)
servoAng = T.u;         % ángulo servo (°)
barraAng = T.v;         % ángulo barra (°)
dt       = mean(diff(time));  % intervalo de muestreo

% Alineación y filtrado
servoAng = servoAng - servoAng(1);          % quitar offset inicial
barraAng = barraAng - barraAng(1);
servoFilt = movmean(servoAng, N);            % media móvil
barraFilt = movmean(barraAng, N);

% --- Graficar señales raw y filtradas ---
figure;
plot(time, servoAng, 'b', time, barraAng, 'r', 'LineWidth',1);
xlabel('Tiempo (s)'); ylabel('Ángulo (°)');
legend('Servo raw','Barra raw'); title('Señales sin filtrar'); grid on;

figure;
plot(time, servoFilt, 'b', time, barraFilt, 'r', 'LineWidth',1);
xlabel('Tiempo (s)'); ylabel('Ángulo (°)');
legend('Servo filt','Barra filt'); title('Señales filtradas'); grid on;

% --- Preparación de datos para identificación ---
% Normalizar amplitud y quitar media
servoId = (servoFilt - mean(servoFilt)) / max(abs(servoFilt));
barraId = (barraFilt - mean(barraFilt)) / max(abs(barraFilt));
data    = iddata(barraId, servoId, dt);

% --- Estimación del modelo OE ---
opt  = oeOptions('Display','off');
sysd = oe(data, [nb nf nk], opt);  % modelo discreto OE
disp('Modelo discreto obtenido:'); disp(sysd);

% Convertir a continuo
sysc = d2c(sysd, 'tustin');

% --- Función de transferencia G(s) ---
[num, den] = tfdata(sysc, 'v');  % vectores de coeficientes
G = tf(num, den);
disp('Función de transferencia continua G(s):'); disp(G);

% --- Parámetros dinámicos ---
K   = dcgain(G);
p   = pole(G);
tau = -1/p(1);
fprintf('Ganancia DC K = %.4f\nConstante de tiempo tau = %.4f s\n', K, tau);

% --- Validación del modelo ---
figure;
compare(data, sysd);
title('Datos vs Modelo Discreto OE'); grid on;

figure;
bode(G);
title('Diagrama de Bode de G(s)'); grid on;

figure;
step(G);
title('Respuesta al Escalón de G(s)'); grid on;

% Fin del script
