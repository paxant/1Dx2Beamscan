function [V_y] = Fy(theta_range, theta, shift_theta, Number_x, Number_y, phi)

    V_y = zeros(length(theta_range), 1); % Инициализация матрицы
    B = phi * (( Number_x - 1 ) / 2 ) ;
   
    for j = 1:length(theta_range)
        % Преобразование углов в пространственные координаты
        u_y = theta_range(j) + shift_theta;
        sum_value = 0; % Инициализация суммы
        for m = 0:Number_y-1
            phase_m = (m - (Number_y - 1 ) / 2);
           
            sum_value = sum_value + exp(1j * pi * (phase_m * u_y - B));
        end
        V_y(j) = (sum_value); % Записываем сумму в матрицу
    end

end