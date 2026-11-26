function [BEAMscan_x,BEAMscan_y, BEAMscan_xy] = Beam1D_F_mod(Nx,Ny, B, t, phi_range, theta_range)
    
    %% ---------------------------------------------------------
    % Попытка посчитать без ковариационных матриц. Работает
    % ---------------------------------------------------------
    length_t = length(t);
    
    Rxx_y = zeros(Ny, 1);
    Rxx_x = zeros(Nx, 1);
    for q = 1:length_t
        for j = 1:Nx
            A = B(:, j, q);
            Rxx_y = Rxx_y + A;
        end
    
        for i = 1:Ny
            A(:, 1) = B(i, :, q);
            Rxx_x = Rxx_x + A;
        end
    end
    
    Rxx_ux = Rxx_x / length_t / Ny; % Нормализация по количеству временных отсчетов
    Rxx_uy = Rxx_y / length_t / Nx;
    
    %% Поиск одномерного спектра для psi_x
    
    BEAMscan_x = zeros(length(phi_range), 1);
    
    for i = 1:length(phi_range)
        for n = 0:Nx-1
            phase_n = (n - (Nx - 1)/2) ;
            Vnm_x(n+1,i) = exp(1j * pi * (phase_n * sind(phi_range(i)) )); % Вектор направленности + (Ny - 1) / 2)
        end

        for j = 1:Nx
            A = B(:, j, q);
            BEAMscan_x(i) = BEAMscan_x(i) + abs(Vnm_x(:, i)' * A).^1; % Расчет спектра
        end
    end
    BEAMscan_x = BEAMscan_x.^2;

    %% Поиск одномерного спектра для psi_y
    BEAMscan_y = zeros(length(theta_range), 1);
    
    for j = 1:length(theta_range)
        for m = 0:Ny-1
            phase_m = (m - (Ny - 1)/2) ;
            Vnm_y(m+1,j) = exp(1j * pi * (phase_m * sind(theta_range(j)) )); % Вектор направленности + (Ny - 1) / 2)
        end

        for i = 1:Nx
            A(:, 1) = B(i, :, q);
            BEAMscan_y(j) = BEAMscan_y(j) + abs(Vnm_x(:, j)' * A).^1; % Расчет спектра
        end        
    end
    BEAMscan_y = BEAMscan_y.^2;

   
    BEAMscan_xy = BEAMscan_x * BEAMscan_y';
    
    BEAMscan_xy_Db = To_dB(BEAMscan_xy);
    BEAMscan_xy_Db(BEAMscan_xy_Db < -60) = -60; 
    GraficFunc(phi_range,theta_range, BEAMscan_xy_Db, 'Угол \theta ^\circ', 'Угол \phi , ^\circ', '2x1D BEAMscan');

    figure;

    % BEAMscan_x
    subplot(2,1,1);
    plot(phi_range, 10*log10(abs(BEAMscan_x) ./ max(abs(BEAMscan_x))), ...
        'b', 'LineWidth', 1.5);
    grid on;
    xlabel('Угол \phi, ^\circ');
    ylabel('Уровень, dB');
    title('BEAMscan_x');
    xlim([-30 30]);
    ylim([-40 0]);
    
    % BEAMscan_y
    subplot(2,1,2);
    plot(theta_range, 10*log10(abs(BEAMscan_y) ./ max(abs(BEAMscan_y))), ...
        'r', 'LineWidth', 1.5);
    grid on;
    xlabel('Угол \theta, ^\circ');
    ylabel('Уровень, dB');
    title('BEAMscan_y');
    xlim([-30 30]);
    ylim([-40 0]);    

end
