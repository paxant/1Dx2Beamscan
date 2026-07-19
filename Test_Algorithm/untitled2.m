clear all; clc; close all;
data = readcell('signals.csv','Delimiter','\t'); % читаем как строки
[m,n] = size(data);
z = zeros(m,n);

for i = 1:m
    for j = 1:n
        str = strrep(data{i,j}, ' ', ''); % удаляем пробелы
        z(i,j) = str2num(str); %#ok<ST2NM>
    end
end


%% Параметры
Nx = 5; Ny = 6; % Размеры антенной решётки
t = 0:0.001:0.001; % Временная шкала
SignalFreq = [ 20 20 ]; % Частота сигнала (Гц)
Ampl = [ 1 1 ] ; % Амплитуда
Targets_x = [ -10 24.75 ];
Targets_y = [ -30 4.75]; %4.75 ];
% Координаты прихода сигнала
Targets_phi = Targets_y/100;
Targets_theta = Targets_x/100;

%Targets_theta = [-0.55,  -0.428, -0.306, -0.183, -0.061, 0.061, 0.183, 0.306, 0.428, 0.55];
%Targets_phi   = [-0.55, -0.428,  -0.306, -0.183, -0.061, 0.061, 0.183, 0.306, 0.428, 0.55];

theta_range = -60:0.5:60; % Угол θ, градусы (101 точка)
phi_range = -60:0.5:60; % Угол φ, градусы (101 точка)

%%
% Формирование сигнала
for k = 1:length(SignalFreq)

    for tt = 1:length(t)
        Signal(k, tt) = Ampl(k) * exp(1j * 2 * pi * SignalFreq(k) * t(tt));
    end
end

for k = 1:length(SignalFreq)
    for tt = 1:length(t)
        if round(real(Signal(k,tt)),3) ~= round(real(z(k,tt)),3) || ...
   round(imag(Signal(k,tt)),3) ~= round(imag(z(k,tt)),3)
            fprintf('Ошибка в сигнале %d на отсчёте %d\n', k, tt);
        end
    end
end


data = readcell('V.csv', 'Delimiter', '\t');  % читаем как строки
[m,n] = size(data);
V_c = zeros(m,n);

for i = 1:m
    for j = 1:n
        str = strrep(data{i,j}, ' ', ''); % удаляем пробелы
        if ~isempty(str)
            V_c(i,j) = str2num(str); %#ok<ST2NM>
        else
            V_c(i,j) = 0;
        end
    end
end










% Формирование направляющей матрицы для целевого сигнала с использованием угловых координат
for k = 1:length(SignalFreq)
    index_matrix = 1;

    for m = 0:Ny - 1

        for n = 0:Nx - 1
            V(index_matrix, k) = exp(1j * (n - (Nx - 1) / 2) * Targets_theta(k) * pi) * ...
                exp(1j * (m - (Ny - 1) / 2) * Targets_phi(k) * pi);
            index_matrix = index_matrix + 1;
        end
    end
end


% Округляем до 3 знаков после запятой
V_c_round = round(V_c, 3);
V_round   = round(V, 3);

% Сравниваем
[m_check, n_check] = size(V_c_round);
for i = 1:m_check
    for j = 1:n_check
        if V_c_round(i,j) ~= V_round(i,j)
            fprintf('Различие в элементе (%d,%d): CSV = %f + %fi, Расчет = %f + %fi\n', ...
                i,j, real(V_c(i,j)), imag(V_c(i,j)), real(V(i,j)), imag(V(i,j)));
        end
    end
end

B = Signal.' * V.';






BEAMscan_x = zeros(length(theta_range), 1);

for i = 1:length(theta_range)
    for n = 0:Nx-1
        phase_n = (n - (Nx-1)/2);
        Vnm_x(n+1,1) = exp(1j * phase_n * theta_range(i)/100 * pi); % Вектор направленности
    end
    BEAMscan_x(i) = Vnm_x' * Rxx_ux * Vnm_x; % Расчет спектра
end


% Поиск одномерного спектра для psi_y
BEAMscan_y = zeros(length(phi_range), 1);

for j = 1:length(phi_range)
     for m = 0:Ny-1
        phase_m = (m - (Ny-1)/2);
        Vnm_y(m+1,1) = exp(1j * phase_m * phi_range(j)/100 * pi); % Вектор направленности
    end
    BEAMscan_y(j) = Vnm_y' * Rxx_uy * Vnm_y; % Расчет спектра
end

%% === Считываем CSV, сгенерированные C-кодом ===
data_x = readcell('BEAMscan_x.csv','Delimiter','\t');
data_y = readcell('BEAMscan_y.csv','Delimiter','\t');

BEAMscan_x_csv = zeros(length(data_x),1);
for i = 1:length(data_x)
    str = strrep(data_x{i},' ',''); % удаляем пробелы
    BEAMscan_x_csv(i) = str2num(str); %#ok<ST2NM>
end
BEAMscan_x = abs(BEAMscan_x) / max(abs(BEAMscan_x(:)));

BEAMscan_y_csv = zeros(length(data_y),1);
for i = 1:length(data_y)
    str = strrep(data_y{i},' ',''); 
    BEAMscan_y_csv(i) = str2num(str); %#ok<ST2NM>
end
BEAMscan_y = abs(BEAMscan_y) / max(abs(BEAMscan_y(:)));
%% === Сравнение до третьего знака ===
for i = 1:length(theta_range)
    csv_val = round(BEAMscan_x_csv(i),3);
    mat_val = round(BEAMscan_x(i),3);
    if csv_val ~= mat_val
        fprintf('Различие в BEAMscan_x(%d): CSV = %0.3f + %0.3fi, MATLAB = %0.3f + %0.3fi\n', ...
            i, real(csv_val), imag(csv_val), real(mat_val), imag(mat_val));
    end
end

for j = 1:length(phi_range)
    csv_val = round(BEAMscan_y_csv(j),3);
    mat_val = round(BEAMscan_y(j),3);
    if csv_val ~= mat_val
        fprintf('Различие в BEAMscan_y(%d): CSV = %0.3f + %0.3fi, MATLAB = %0.3f + %0.3fi\n', ...
            j, real(csv_val), imag(csv_val), real(mat_val), imag(mat_val));
    end
end

BEAMscan =  flip(BEAMscan_x_csv) * BEAMscan_y_csv';
BEAMscan = 10*log10(BEAMscan);
BEAMscan(BEAMscan < -60) = -60;

function [] = GraficFunc(theta_range,phi_range, Matrix, xlable_Name, ylabel_Name, title_Name)
% Визуализация спектра по угловым координатам
figure;
imagesc(theta_range, phi_range, Matrix);
colorbar;
xlabel(xlable_Name);
ylabel(ylabel_Name);
title(title_Name);
axis xy;
grid on;
colormap('jet');
end

GraficFunc(theta_range, phi_range,  BEAMscan, 'Угол \theta (градусы)', 'Угол \phi (градусы)', 'Пространственный спектр BeamScan');


