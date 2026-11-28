clear all; clc; close all; 

%% Параметры для проверки результата

D1x2_to_1_snapshot = 0;
D2_enable = 1; %Расчет ДПС
D1_enable = 1; %Расчет ОПС и МПС по ним
D1Comp_enable = 1; %Компенсация искажений МПС

PlotArrayEnable = 0;          %Рисовать элементы АР
PlotD1Enable = 1;    %Отображать 2*1Д спектр

%% Параметры 

Nx = 25; Ny = 15; % Размеры антенной решётки 
t = 0; % : 0.0001: 0.01; % Временная шкала 

Ampl = [1 1]; % Амплитуда 
SignalFreq = [1000 1100]; % Частота сигнала (Гц)

ro = 0.75;      %Коэффициент взаимной корреляции
fi = [0 acosd(ro) -acos(ro)] * pi / 180; %начальная фаза
fi = [0 acosd(ro)] * pi / 180; %начальная фаза
%fi = 2 * pi * rand(size(Ampl));   %Случайная начальная фаза
Ampl = Ampl .* exp(1j * fi);      %Комплексная амплитуда

% Углы прихода сигнала
%Targets_x = [18.546, -11.65];
%Targets_y = [-15.67, 15.76];

Targets_x = [5, -19.02];         %Угол φ, азимут
Targets_y = [5, -7];       %Угол θ, угол места

SNR = 10000;
%Диапазон углов, в пределах которых расчитывается ДПС
theta_range = -30:0.5:30; % Угол θ, градусы 
phi_range   = -30:0.5:30; % Угол φ, градусы 



%% Формирование матрицы направленности

[B] = Modeling_Rentagular(SignalFreq,t,Ampl,Nx,Ny,Targets_x,Targets_y, SNR, PlotArrayEnable);

%% Расчёт одномерных ПС

if D1_enable
    [BEAMscan_x,BEAMscan_y,BEAMscan_xy] = Beam1D_F(Nx, Ny, B, t, phi_range, theta_range);
    
    BEAMscan_xy_Db = To_dB(BEAMscan_xy);
    BEAMscan_xy_Db(BEAMscan_xy_Db < -60) = -60; 
    GraficFunc(phi_range,theta_range, BEAMscan_xy_Db, 'Угол \phi , ^\circ', 'Угол \theta ^\circ', '2x1D BEAMscan', PlotD1Enable);
        
    %Plot_2xBeamScan1D(BEAMscan_x,BEAMscan_y, phi_range, theta_range);

    %% Формирование компенсации
    
    [P_cross,P_cross_full] = additional_P(Targets_x,Targets_y, phi_range, theta_range, Nx, Ny, length(Targets_x), ro, Ampl);
    %Вычисление компенсации

    BEAMscan_xy_comp_Db = To_dB(BEAMscan_xy - P_cross);
    %BEAMscan_xy_Db = To_dB(abs(P_cross));
   
    GraficFunc(phi_range,theta_range, BEAMscan_xy_comp_Db, 'Угол \phi , ^\circ', 'Угол \theta ^\circ', 'ПС с восстановленной структурой', D1Comp_enable);
    
end

%% Расчет двумерного ПС проверки
if D2_enable
    [BEAMscan_2D] = Beam2D_F(B,phi_range, theta_range, Ny, Nx);
    
    BEAMscan_2D_Db = To_dB(BEAMscan_2D);
    GraficFunc(phi_range,theta_range, BEAMscan_2D_Db, 'Угол \phi , ^\circ', 'Угол \theta ^\circ', '2D BEAMscan', D2_enable);
end 

RadiationPatternAnalyz;