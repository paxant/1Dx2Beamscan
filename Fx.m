function [V_x] = Fx(phi_range, shift_phi, Number_x, Number_y)
    
    V_x = zeros(1, length(phi_range), 1); % Инициализация матрицы
    %A = ( Number_y - 1 ) / 2;
    for i = 1:length(phi_range)
        % Преобразование углов в пространственные координаты
        u_x = sind(phi_range(i)) + sind(shift_phi); 
        sum_value = 0; % Инициализация суммы
        for n = 0:Number_x-1
            phase_n = (n - ( Number_x -1 ) / 2);
            
            sum_value = sum_value + exp(-1j * pi * ( phase_n * u_x ) );
        end
        V_x(1, i) = (sum_value); 
    end
    
end

