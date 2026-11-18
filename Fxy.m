function [F] = Fxy(phi_range, theta_range, shift_phi, shift_theta, Number_x, Number_y)
    
    V_x = zeros(1, length(phi_range), 1); % Инициализация матрицы
    %A = ( Number_y - 1 ) / 2;
    for i = 1:length(phi_range)
        for j = 1: length(theta_range)
        % Преобразование углов в пространственные координаты
        u_x = sind(phi_range(i)) + sind(shift_phi); 
        u_y = sind(theta_range(j)) + sind(shift_theta);
        sum_value = 0; % Инициализация суммы
        for n = 0:Number_x-1
            phase_n = (n - ( Number_x -1 ) / 2);
            for m = 0: Number_y-1
                
                phase_m = (m - (Number_y - 1 ) / 2);
                sum_value = sum_value + exp(-1j * pi * ( phase_n * u_x + phase_m * u_y) );
        
            end
        end

        F(j, i) = (sum_value); 
        end
    end
    
end

