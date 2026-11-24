clear all; clc; close all; 

%% Параметры для проверки результата

D1x2_to_1_snapshot = 0;
D2_enable = 0;
D1_enable = 1;

PlotArrayEnable = 0;          %Рисовать элементы АР

%% Параметры 

Nx = 25; Ny = 15; % Размеры антенной решётки 
t = 0;%:0.001:0.01; % Временная шкала 
SignalFreq = [0 90]; % Частота сигнала (Гц) 

Ampl = [1 1]; % Амплитуда 
ro = 1;      %Коэффициент взаимной корреляции

fi = [0 acosd(ro)] * pi / 180; %начальная фаза
Ampl = Ampl .* exp(1j * fi);


Targets_x = [18.546, -11.65];
Targets_y = [-15.67, 15.76];

% Targets_x = [5, -7];         %Угол φ
% Targets_y = [10, -14];       %Угол θ
SNR = 10000;

theta_range = -30:0.5:30; % Угол θ, градусы 
phi_range   = -30:0.5:30; % Угол φ, градусы 

%% Координаты прихода сигнала

%% Формирование матрицы направленности

[B] = Modeling_Rentagular(SignalFreq,t,Ampl,Nx,Ny,Targets_x,Targets_y, SNR, PlotArrayEnable);

%% Расчёт одномерных ПС

if D1_enable
    [BEAMscan_x,BEAMscan_y,BEAMscan_xy] = Beam1D_F(Nx, Ny, B, t, phi_range, theta_range);
    
    BEAMscan_xy_Db = To_dB(BEAMscan_xy);
    BEAMscan_xy_Db(BEAMscan_xy_Db < -60) = -60; 
    GraficFunc(phi_range,theta_range, BEAMscan_xy_Db, 'Угол \phi , ^\circ', 'Угол \theta ^\circ', '2x1D BEAMscan');
        
    %Plot_2xBeamScan1D(BEAMscan_x,BEAMscan_y, phi_range, theta_range);

%% Формирование компенсации
    [P_cross,P_cross_full] = additional_P(Targets_x,Targets_y, phi_range, theta_range, Nx, Ny, length(Targets_x), ro);
%% Вычисоение компенсации

    BEAMscan_xy_Db = To_dB(abs(BEAMscan_xy) - abs(P_cross));
    GraficFunc(phi_range,theta_range, BEAMscan_xy_Db, 'Угол \phi , ^\circ', 'Угол \theta ^\circ', 'ПС с восстановленной структурой');
end

%% Расчет двумерного ПС проверки
if D2_enable
    [BEAMscan_2D] = Beam2D_F(B,phi_range, theta_range, Ny, Nx);
    
    BEAMscan_xy_Db = To_dB(BEAMscan_2D);
    GraficFunc(phi_range,theta_range, BEAMscan_xy_Db, 'Угол \phi , ^\circ', 'Угол \theta ^\circ', '2D BEAMscan');
end 

RadiationPatternAnalyz;