function [P_cross,P_cross_full] = additional_P(Targets_x,Targets_y, phi_range, theta_range, Nx, Ny, Number_of_targets, ro)
    
    P_cross_full = zeros(numel(theta_range), numel(phi_range));
    P_cross = zeros(numel(theta_range), numel(phi_range));
    DFx = zeros(Number_of_targets, numel(phi_range));
    DFy = zeros(Number_of_targets, numel(theta_range));
    
    for i=1:length(Targets_x)

        DFx(i, :) = Fx( (phi_range), (Targets_x(i)), Nx, Ny ) ;
        
        DFy(i, :) = Fy((theta_range), (Targets_y(i)), Nx, Ny ); 
    
    end

    for i = 1:Number_of_targets
        for j = 1:Number_of_targets  
            if j ~= i
                P_cross = P_cross + ( ( ( DFy(j, :) )' * DFx(i, :) ).^2 );
           end
        end
    end
    
    if ro == 1
        for i = 1:Number_of_targets
            for j = i+1:Number_of_targets
    
                P_cross = P_cross - (2 * ( ( DFy(i, :) )' * DFx (i, :) ) .* ( ( DFy(j, :) )' * DFx(j, :) ) );
    
            end
        end
    end
end

