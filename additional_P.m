function [P_cross,P_cross_full] = additional_P(Targets_x,Targets_y, phi_range, theta_range, Nx, Ny, Number_of_targets, t)
    
    P_cross_full = zeros(numel(theta_range), numel(phi_range));
    P_cross = zeros(numel(theta_range), numel(phi_range));
    DFx = zeros(numel(phi_range), Number_of_targets);
    DFy = zeros(numel(theta_range), Number_of_targets);
    
    for i=1:length(Targets_x)

        DFx(:, i) = Fx( (phi_range), (Targets_x(i)), Nx, Ny ) ;
        
        DFy(:, i) = Fy((theta_range), (Targets_y(i)), Nx, Ny ); 
    
    end

    for i = 1:Number_of_targets
        for j = 1:Number_of_targets  
            if j ~= i
                P_cross = P_cross + rot90(( flip(DFx(:, j)) * DFy(:, i)' ).^2);
           end
        end
    end
    
    % for i = 1:Number_of_targets
    %     for j = 1:Number_of_targets  
    %             P_cross_full = P_cross_full + rot90(( flip(DFx(:, j)) * DFy(:, i)' )).^2 ;
    %     end
    % end
    % 
    if numel(t) == 1
        for i = 1:Number_of_targets
            for j = i+1:Number_of_targets
    
                % P_cross_full = P_cross_full + rot90(2 * (flip(DFx(:, i)) * DFy(:, i)') .* (flip(DFx(:, j)) * DFy(:, j)'));
    
                P_cross = P_cross - rot90(2 * (flip(DFx(:, i)) * DFy(:, i)') .* (flip(DFx(:, j)) * DFy(:, j)'));
    
            end
        end
    end
end

