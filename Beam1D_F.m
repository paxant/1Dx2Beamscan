function [BEAMscan_x,BEAMscan_y, BEAMscan_xy] = Beam1D_F(Nx,Ny, B, length_t, phi_range, theta_range)
    
    % V_x = zeros(Nx, 1);
    % V_y = zeros(Ny, 1);   
    % 
    % for j = 1:Nx
    % 
    %     % Координата столбца относительно фазового центра
    %     dx = (j-1) - (Nx-1)/2;
    % 
    %     % Фазовый сдвиг столбца к фазовому центру
    %     shift_phase_x = exp(-1j * pi * dx);
    %     AAAA = B(:,j);
    %     % Перенос столбца в фазовый центр и сложение
    %     V_y = V_y + AAAA .* shift_phase_x;
    % end
    % for i = 1:Ny
    % 
    %     % Координата строки относительно фазового центра
    %     dy = (i-1) - (Ny-1)/2;
    % 
    %     % Фазовый сдвиг строки к фазовому центру
    %     shift_phase_y = exp(-1j * pi * dy);
    %     AAAA = B(i,:);
    %     % Перенос строки в фазовый центр и сложение
    %     V_x = V_x + AAAA .* shift_phase_y;
    % end
    % 
    % V_x = V_x.';

    %% ---------------------------------------------------------
    % Расчет ковариационных матриц
    % ---------------------------------------------------------

    %% Рассчитываем корреляционные матрицы
    Rxx_y = zeros(Ny);
    Rxx_x = zeros(Nx);
    
    % Rxx_x = V_x * V_x';
    % Rxx_y = V_y * V_y';
    % 
    % Rxx_ux = Rxx_x / length_t; % Нормализация по количеству временных отсчетов
    % Rxx_uy = Rxx_y / length_t;

    for j = 1:Nx
        A = B(:, j);
        Rxx_y = Rxx_y + ( A * A' );
    end

    for i = 1:Ny
        A = B(i, :).';
        Rxx_x = Rxx_x + ( A * A' );
    end
    
    Rxx_ux = Rxx_x / length_t / Nx; % Нормализация по количеству временных отсчетов
    Rxx_uy = Rxx_y / length_t / Ny;
    
    % Поиск одномерного спектра для psi_x
    
    BEAMscan_x = zeros(length(phi_range), 1);
    
    for i = 1:length(phi_range)
        for n = 0:Nx-1
            phase_n = (n - (Nx-1)/2) ;
            Vnm_x(n+1,1) = exp(1j * pi * phase_n * phi_range(i)/100  ); % Вектор направленности
        end
        BEAMscan_x(i) = Vnm_x' * Rxx_ux * Vnm_x; % Расчет спектра
    end
    
    % Поиск одномерного спектра для psi_y
    BEAMscan_y = zeros(length(theta_range), 1);
    
    for j = 1:length(theta_range)
        for m = 0:Ny-1
            phase_m = (m - (Ny-1)/2) ;
            Vnm_y(m+1,1) = exp(1j * pi * phase_m * theta_range(j)/100); % Вектор направленности
        end
        BEAMscan_y(j) = Vnm_y' * Rxx_uy * Vnm_y; % Расчет спектра
    end
    
    BEAMscan_xy = (BEAMscan_y) * (BEAMscan_x)';
    % BEAMscan_xy = rot90(BEAMscan_xy);
    
     % BEAMscan_x = abs(BEAMscan_x) / max(abs(BEAMscan_x)); 
     % BEAMscan_y = abs(BEAMscan_y) / max(abs(BEAMscan_y)); 
     % BEAMscan_xy = abs(BEAMscan_xy) / max(abs(BEAMscan_xy(:)));

end

