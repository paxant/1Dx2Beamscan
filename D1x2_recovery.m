clear all; clc; close all; 

%% Параметры для проверки результата

D1x2_to_1_snapshot = 0;
D2_enable = 0;
D1_enable = 0;

%% Параметры 

Nx = 25; Ny = 25; % Размеры антенной решётки 
t = 0:0.001:0.01; % Временная шкала 
SignalFreq = [20 130]; % Частота сигнала (Гц) 
Ampl = [1 1]; % Амплитуда 

Targets_x = [18.546, -11.65];
Targets_y = [-15.67, 15.76];

theta_range = -30:0.5:30; % Угол θ, градусы 
phi_range   = -30:0.5:30; % Угол φ, градусы 

%% Координаты прихода сигнала

%% Формирование матрицы направленности

[B] = Modeling_Rentagular(SignalFreq,t,Ampl,Nx,Ny,Targets_x,Targets_y);

%% Расчёт одномерных ПС

if D1_enable
    [BEAMscan_x,BEAMscan_y,BEAMscan_xy] = Beam1D_F(Nx, Ny, B, t, phi_range, theta_range);
    
    BEAMscan_xy_Db = To_dB(BEAMscan_xy);
    GraficFunc(phi_range,theta_range, BEAMscan_xy_Db, 'Угол Theta (deg)', 'Угол Phi (deg)', 'BEAMscan_xy ');

%% Формирование компенсации
    if D1x2_to_1_snapshot
        [P_cross,P_cross_full] = additional_P(Targets_x,Targets_y, phi_range, theta_range, Nx, Ny, length(Targets_x), 1);
    else
        [P_cross,P_cross_full] = additional_P(Targets_x,Targets_y, phi_range, theta_range, Nx, Ny, length(Targets_x), t);
    end
%% Вычисоение компенсации

    BEAMscan_xy_Db = To_dB(abs(BEAMscan_xy) - abs(P_cross));
    GraficFunc(phi_range,theta_range, BEAMscan_xy_Db, 'Угол \theta ^\circ', 'Угол \phi , ^\circ', 'ПС с восстановленной структурой');
end

%% Расчет двумерного ПС проверки
if D2_enable
    [BEAMscan_2D] = Beam2D_F(B,phi_range, theta_range, Ny, Nx);
    
    BEAMscan_xy_Db = To_dB(BEAMscan_2D);
    GraficFunc(phi_range,theta_range, BEAMscan_xy_Db, 'Угол Theta (deg)', 'Угол Phi (deg)', 'BEAMscan_2D ');
end 