function [V_y] = Fy(theta_range, shift_theta, Number_x, Number_y)

    V_y = zeros(1, length(theta_range), 1); % Инициализация матрицы
    %B = ( Number_x - 1 ) / 2;
   
    for j = 1:length(theta_range)
        % Преобразование углов в пространственные координаты
        u_y = sind(theta_range(j)) - sind(shift_theta);
        sum_value = 0; % Инициализация суммы
        for m = 0:Number_y-1
            phase_m = (m - (Number_y - 1 ) / 2);
           
            sum_value = sum_value + exp(1j * pi * (phase_m * u_y));
        end
        % Если бы в матлабе данные хранились нормально, как во всех остальных программах, то при повороте
        % матриц не приходилось бы использовать flip(). Этот костыль
        % устраняет несостыковку при повороте вектора через " ' "
        V_y(1, length(theta_range) - j+1) = (sum_value); 
    end

end