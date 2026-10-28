function [BEAMscan] = Beam2D_F(B,X_targets, Y_targets, Ny, Nx)
    
    Nsnap = size(B, 3);
    M = Ny * Nx;
    Rxx = complex(zeros(M, M));

    for q = 1:Nsnap
        b = reshape(B(:, :, q), [], 1);
        Rxx = Rxx + b * b';
    end

    Rxx = Rxx / Nsnap;

    %% ---------------------------------------------------------
    % Формирование матрицы направленности Vnm
    % ---------------------------------------------------------

    Vnm = zeros(length(Y_targets), length(X_targets), Ny, Nx);

    for i = 1:length(X_targets)
        for j = 1:length(Y_targets)
            % Преобразование углов в пространственные координаты
            u_x = sind(X_targets(i));
            u_y = sind(Y_targets(j));
            for n = 0:Nx - 1
                for m = 0:Ny - 1
                    phase_n = (n - (Nx - 1)/2);
                    phase_m = (m - (Ny - 1)/2);
                    Vnm(j, i, m + 1, n + 1) = exp(1j * pi * (phase_n .* (u_x) + phase_m .* (u_y)));
                end
            end
        end
    end

    BEAMscan = zeros(length(Y_targets), length(X_targets));

    for i = 1:length(X_targets)
        for j = 1:length(Y_targets)
            % Извлекаем вектор направленности a(θ,φ)
            a = reshape(Vnm(j, i, :, :), [], 1);
            % Вычисляем спектральную мощность
            BEAMscan(j, i) = (a' * Rxx * a);
        end
    end
end