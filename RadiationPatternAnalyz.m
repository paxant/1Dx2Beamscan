%% Программа построения ДПС и МПС по диаграммам направленности
PlotOPSenable = 0;
PlotDPSenable = 1;
%Диаграммы
Fx1 = Ampl(1) * Fx( (phi_range), (Targets_x(1)), Nx, Ny ) ;
Fx2 = Ampl(2) * Fx( (phi_range), (Targets_x(2)), Nx, Ny ) ;
Fy1 = Ampl(1) * Fy( (theta_range), (Targets_y(1)), Nx, Ny ) ;
Fy2 = Ampl(2) * Fy( (theta_range), (Targets_y(2)), Nx, Ny ) ;

%% Одномерные пространственные спектры
if PlotOPSenable == 1
    Fx12 = abs(Fx1).^2 + abs(Fx2).^2; 
    Fy12 = abs(Fy1).^2 + abs(Fy2).^2; 
    
    Fx12 = Fx12.^1;% - 2 * abs(Fx1) .* abs(Fx2);
    Fy12 = Fy12.^1;% - 2 * abs(Fy1) .* abs(Fy2);
    
    Fx_dB = 10 * log10(abs(Fx12) / max(max(abs(Fx12))));
    Fy_dB = 10 * log10(abs(Fy12) / max(max(abs(Fy12))));
    
    figure
    plot(phi_range, Fx_dB, phi_range, 10*log10(abs(BEAMscan_x) ./ max(abs(BEAMscan_x))));
    grid on;
    figure
    plot(theta_range, Fy_dB, theta_range, 10*log10(abs(BEAMscan_y) ./ max(abs(BEAMscan_y))))
    grid on;
end


%% Двумерный пространственный спектр

DPS = (Fy1' * Fx1).^2 + (Fy2' * Fx2).^2 + 2 * (Fy1' * Fx1) .* (Fy2' * Fx2) * ro;

%Нормировка 
DPSn = DPS / max(max(abs(DPS)));
%ДПС в дБ
DPSn_dB = 10 * log10(abs(DPSn));
DPSn_dB(DPSn_dB < -60) = -60; 
if PlotDPSenable == 1
    GraficFunc(phi_range,theta_range, DPSn_dB, 'Угол \theta ^\circ', 'Угол \phi , ^\circ', 'RadiationPatterns');
end

%% МПС (совокупность двух одномерных)
MPS = (abs(Fy1).^2 + abs(Fy2).^2)'  * (abs(Fx1).^2 + abs(Fx2).^2);

%Нормировка 
MPSn = MPS / max(max(abs(MPS)));
%МПС в дБ
MPSn_dB = 10 * log10(abs(MPSn));
MPSn_dB(MPSn_dB < -60) = -60; 
GraficFunc(phi_range,theta_range, MPSn_dB, 'Угол \theta ^\circ', 'Угол \phi , ^\circ', 'RadiationPatterns MPS');

%% Компенсация
Comp = (abs(Fy1)'.^2 * abs(Fx2).^2) + (abs(Fy2)'.^2 * abs(Fx1).^2) - 2 * (Fy1' * Fx1) .* (Fy2' * Fx2) * ro;

%Такая запись компенсации дает 2D спектр
%Comp = (abs(Fy1)'.^2 * abs(Fx2).^2) + (abs(Fy2)'.^2 * abs(Fx1).^2) - 2 * (Fy1' * Fx1) .* (Fy2' * Fx2) * ro;


%Компенсация искажений. MPS_c = DPS. 
MPS_c = MPS - Comp;
%MPS_c = Comp;
%MPS_c = MPS - (Comp).';
%Нормировка
MPS_cn = MPS_c / max(max(abs(MPS_c)));
MPS_cn_dB = 10 * log10(abs(MPS_cn));
MPS_cn_dB(MPS_cn_dB < -60) = -60; 
GraficFunc(phi_range,theta_range, MPS_cn_dB, 'Угол \theta ^\circ', 'Угол \phi , ^\circ', 'RadiationPatterns MPS_c');

%Дополнительное подавление боковых лепестков
Comp2 = 2 * (abs(Fy1)' * abs(Fx1)) .* (abs(Fy2)' * abs(Fx2)) * abs(ro);
%УБЛ MPS_c2 < DPS ! Причем очень интенсивно БЛ давятся в окрестности ложных
%объектов. Я это проверял для разных положений объектов и ро, везде есть
%выигрыш.
MPS_c2 = MPS_c - Comp2;
%Нормировка
MPS_c2n = MPS_c2 / max(max(abs(MPS_c2)));
MPS_c2n_dB = 10 * log10(abs(MPS_c2n));
MPS_c2n_dB(MPS_c2n_dB < -60) = -60; 
GraficFunc(phi_range,theta_range, MPS_c2n_dB, 'Угол \theta ^\circ', 'Угол \phi , ^\circ', 'RadiationPatterns MPS_c_2');