function [BEAMscan_x,BEAMscan_y, BEAMscan_xy] = Beam1D_F(Nx,Ny, B, t, phi_range, theta_range)
    
    %% ---------------------------------------------------------
    % Расчет ковариационных матриц
    % ---------------------------------------------------------
    length_t = length(t);
    %% Рассчитываем корреляционные матрицы
    Rxx_y = zeros(Ny);
    Rxx_x = zeros(Nx);
    for q = 1:length_t
        for j = 1:Nx
            A = B(:, j, q);
            Rxx_y = Rxx_y + ( A * A' );
        end
    
        for i = 1:Ny
            A(: ,1) = B(i, :, q); 
            %Убран поворот матрицы
            Rxx_x = Rxx_x + ( A * A' );
        end
    end
    
    % Это лучше не трогать. Отсутсвие нормировки приведет к неправильной
    % компенсации из-за различия в амплитудах
    Rxx_ux = Rxx_x / length_t / Ny; % Нормализация по количеству временных отсчетов
    Rxx_uy = Rxx_y / length_t / Nx;

    
    % Поиск одномерного спектра для psi_x
    
    BEAMscan_x = zeros(length(phi_range), 1);
    
    for i = 1:length(phi_range)
        for n = 0:Nx-1
            phase_n = (n - (Nx - 1)/2) ;
            Vnm_x(n+1,1) = exp(1j * pi * (phase_n * sind(phi_range(i)) )); % Вектор направленности + (Ny - 1) / 2)
        end
        BEAMscan_x(i) = Vnm_x' * Rxx_uy * Vnm_x; % Расчет спектра
    end
    
    % Поиск одномерного спектра для psi_y
    BEAMscan_y = zeros(length(theta_range), 1);
    
    for j = 1:length(theta_range)
        for m = 0:Ny-1
            phase_m = (m - (Ny - 1)/2) ;
            Vnm_y(m+1,1) = exp(1j * pi * (phase_m * sind(theta_range(j)) )); % Вектор направленности + (Ny - 1) / 2)
        end
        BEAMscan_y(j) = Vnm_y' * Rxx_ux * Vnm_y; % Расчет спектра
    end
    
    BEAMscan_xy = (BEAMscan_x) * (BEAMscan_y)'; %убрал теперь не нужный флип

end
