clear all; clc; close all; 

%% Параметры 

Nx = 30; Ny = 30; % Размеры антенной решётки 
t = 0.001; % Временная шкала 
SignalFreq = [20 20]; % Частота сигнала (Гц) 
Ampl = [1 1]; % Амплитуда 
SNR = -120000; % ОСШ, дБ

% Targets_x   = [-10, 15.75];
% Targets_y   = [-24.678, 14.75];

Targets_x = [ -10 25 ];
Targets_y = [ -20 15 ];

theta_range = -60:0.5:60; % Угол θ, градусы 
phi_range = -60:0.5:60; % Угол φ, градусы 

SNR = 10^(SNR / 20);

%% Координаты прихода сигнала

Targets_phi = Targets_x / 100;
Targets_theta = Targets_y / 100;

%% Формирование матрицы направленности

[B] = Modeling_Rentagular(SignalFreq,t,Ampl,Nx,Ny,Targets_phi,Targets_theta,SNR);

%% Расчёт одномерных ПС

[BEAMscan_x,BEAMscan_y,BEAMscan_xy] = Beam1D_F(Nx,Ny,B,length(t),phi_range,theta_range);

%% Формирование двумерного ПС
BEAMscan_xy_dB = abs(BEAMscan_xy) / max((abs(BEAMscan_xy(:))));
BEAMscan_xy_dB = 10*log10(BEAMscan_xy_dB);
BEAMscan_xy_dB(BEAMscan_xy_dB < -60) = -60;
GraficFunc(phi_range,theta_range, BEAMscan_xy_dB, 'Угол Theta (deg)', 'Угол Phi (deg)', '2x1D BeamScan');

P_cross = zeros(numel(phi_range),numel(theta_range));
N = length(SignalFreq);

% Расчет через две одномерных ДН.

for i = 1:N
    for j = 1:N
        Fx_i = Fx(phi_range/100, Targets_phi(i), Targets_phi(i), Nx, Ny, Targets_theta(i)); 
        if j ~= i
            Fy_j = Fy(theta_range/100, Targets_theta(j), Targets_theta(j), Nx, Ny, Targets_phi(j)); 
            P_cross = P_cross + rot90(rot90(( Fy_j * Fx_i' )));
       end
    end
end

% Рассчитаем через двумерных ДН.
 % P_cross = F(theta_range, phi_range, Targets_phi , Targets_theta, ones(1, N), Nx, Ny);

P_cross_dB = abs(P_cross) / max((abs(P_cross(:))));
P_cross_dB = abs(P_cross_dB).^2;
P_cross_dB = 10*log10(P_cross_dB);
P_cross_dB(P_cross_dB < -60) = -60;
GraficFunc(phi_range,theta_range, P_cross_dB, 'Угол Theta (deg)', 'Угол Phi (deg)', 'P_cross');

%% Формирование итогового двумерного ПС
% BEAMscan_xy = abs(BEAMscan_xy) / max(abs(BEAMscan_xy(:)));
%AAAAAAAAAA = max(P_cross,0);
AAAAAAAAAA = abs(P_cross);
BEAMscan_xy_Db = abs(BEAMscan_xy) - abs(AAAAAAAAAA).^2;
BEAMscan_xy_Db = BEAMscan_xy_Db / max(BEAMscan_xy_Db(:));
BEAMscan_xy_Db = abs(BEAMscan_xy_Db);
BEAMscan_xy_Db = 10*log10(BEAMscan_xy_Db);
BEAMscan_xy_Db(BEAMscan_xy_Db < -60) = -60;

%% =========================================================
% Нормировка одномерного ПС 

BeamX = abs(BEAMscan_x) / max(abs(BEAMscan_x(:)));
BeamY = abs(BEAMscan_y) / max(abs(BEAMscan_y(:)));

GraficFunc(phi_range,theta_range, BEAMscan_xy_Db, 'Угол Theta (deg)', 'Угол Phi (deg)', '2x1D BeamScan - P_cross ');

%% Расчет двумерного ПС
[BEAMscan_2D_] = Beam2D_F(B,phi_range, theta_range, Ny, Nx);

BEAMscan_2D = abs(BEAMscan_2D_) / max(abs(BEAMscan_2D_(:)));
BEAMscan_2D = 10*log10(BEAMscan_2D);
BEAMscan_2D(BEAMscan_2D < -60) = -60;

GraficFunc(phi_range,theta_range,BEAMscan_2D, 'Угол Theta (deg)', 'Угол Phi (deg)', '2D BeamScan');

[BEAMscan_2D_] = Beam2D_F(B,phi_range, theta_range, Ny, Nx);

BEAMscan_2D = BEAMscan_2D_ + abs(AAAAAAAAAA).^2;

BEAMscan_2D = abs(BEAMscan_2D) / max(abs(BEAMscan_2D(:)));
BEAMscan_2D = 10*log10(BEAMscan_2D);
BEAMscan_2D(BEAMscan_2D < -60) = -60;

GraficFunc(phi_range,theta_range,BEAMscan_2D, 'Угол Theta (deg)', 'Угол Phi (deg)', '2D BeamScan + Pross');