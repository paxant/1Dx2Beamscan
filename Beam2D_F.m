function [BEAMscan] = Beam2D_F(B,X_targets, Y_targets, Ny, Nx)
    
    B = B(:);
    Rxx = B * B';
    
    %% ---------------------------------------------------------
    % Формирование матрицы направленности Vnm
    % ---------------------------------------------------------

    Vnm = zeros(length(Y_targets), length(X_targets), Ny, Nx);

    for i = 1:length(X_targets) 
        for j = 1:length(Y_targets) 
            % Преобразование углов в пространственные координаты 
            u_x = X_targets(i)/(100); 
            u_y = Y_targets(j)/(100); 
            for n = 0:Nx - 1 
                for m = 0:Ny - 1 
                    phase_n = (n - (Nx - 1) / 2); 
                    phase_m = (m - (Ny - 1) / 2); 
                    Vnm(j, i, m + 1, n + 1) = exp(1j * pi * (phase_n .* (u_x) + phase_m .* (u_y))); 
                end 
            end 
        end 
    end 
 
    BEAMscan = zeros(length(Y_targets), length(X_targets));

    for i = 1:length(X_targets) 
        for j = 1:length(Y_targets) 
            % Извлекаем вектор направленности a(θ,φ)
            a = reshape(permute(Vnm(j, i, :, :), [4, 3, 1, 2]), [], 1); 
            % Вычисляем спектральную мощность
            BEAMscan(j, i) = (a' * Rxx * a); 
        end 
    end 
    BEAMscan = rot90(BEAMscan);
    BEAMscan = flipud(BEAMscan);
end

