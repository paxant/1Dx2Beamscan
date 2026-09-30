function [V_x] = Fx(phi_range, phi, shift_phi, Number_x, Number_y, theta)
    
    V_x = zeros(length(phi_range), 1); % Инициализация матрицы
    A =  theta * (( Number_y -1 ) / 2  );
    for i = 1:length(phi_range)
        % Преобразование углов в пространственные координаты
        u_x = ( phi_range(i) ) + shift_phi; 
        sum_value = 0; % Инициализация суммы
        for n = 0:Number_x-1
            phase_n = (n - ( Number_x -1 ) / 2);
            
            sum_value = sum_value + exp(1j * pi * ( phase_n * u_x - A));
        end
            %% Для большей точности восстановления заменить 2*pi на 1.97822331045*pi
        V_x(i) = (sum_value); 
    end
    
end
