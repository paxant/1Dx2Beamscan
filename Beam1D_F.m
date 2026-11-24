function [BEAMscan_x,BEAMscan_y, BEAMscan_xy] = Beam1D_F(Nx,Ny, B, t, phi_range, theta_range)
    
    %% ---------------------------------------------------------
    % Расчет ковариационных матриц
    % ---------------------------------------------------------
    length_t = length(t);
    %% Рассчитываем корреляционные матрицы
    C_y = zeros(Ny);
    C_x = zeros(Nx);
    for q = 1:length_t
        for j = 1:Nx
            A = B(:, j, q);
            C_y = C_y + ( A * A' )';
        end
    
        for i = 1:Ny
            A = rot90(flip(B(i, :, q))); % прошлая конструкция не работала при разных Ny и Nx
            %Убран поворот матрицы
            C_x = C_x + ( A * A' );
        end
    end
    
    % Это лучше не трогать. Отсутсвие нормировки приведет к неправильной
    % компенсации из-за различия в амплитудах
    C_x = C_x / length_t / Ny; % Нормализация по количеству временных отсчетов
    C_y = C_y / length_t / Nx;

    
    % Поиск одномерного спектра для φ
    
    BEAMscan_x = zeros(1, length(phi_range));
    
    for i = 1:length(phi_range)
        for n = 0:Nx-1
            phase_n = (n - (Nx - 1)/2) ;
            Vnm_x(n+1,i) = exp(1j * pi * (phase_n * sind(phi_range(i)) )); % Вектор направленности + (Ny - 1) / 2)
        end
        BEAMscan_x(1, i) = Vnm_x(:, i)' * C_x * Vnm_x(:, i); % Расчет спектра
    end
    
    % Поиск одномерного спектра для θ
    BEAMscan_y = zeros(1, length(theta_range));

    for j = 1:length(theta_range)
        for m = 0:Ny-1
            phase_m = (m - (Ny - 1)/2) ;
            Vnm_y(m+1,j) = exp(1j * pi * (phase_m * sind(theta_range(j)) )); % Вектор направленности + (Ny - 1) / 2)
        end
        BEAMscan_y(1, j) = Vnm_y(:, j)' * C_y * Vnm_y(:, j); % Расчет спектра
    end
    
    BEAMscan_xy = (BEAMscan_y)' * (BEAMscan_x); %убрал теперь не нужный флип

end
