function [V_y] = Fy(theta_range, shift_theta, Number_x, Number_y)

    V_y = zeros(1, length(theta_range), 1); % Инициализация матрицы
    %B = ( Number_x - 1 ) / 2;
   
    for j = 1:length(theta_range)
        % Преобразование углов в пространственные координаты
        u_y = sind(theta_range(j)) + sind(shift_theta);
        sum_value = 0; % Инициализация суммы
        for m = 0:Number_y-1
            phase_m = (m - (Number_y - 1 ) / 2);
           
            sum_value = sum_value + exp(-1j * pi * (phase_m * u_y));
        end
        V_y(1, j) = (sum_value); 
    end

end