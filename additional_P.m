function [P_cross,P_cross_full] = additional_P(Targets_x,Targets_y, phi_range, theta_range, Nx, Ny, Number_of_targets, ro, Ampl)
    
    P_cross_full = zeros(numel(theta_range), numel(phi_range));
    P_cross = zeros(numel(theta_range), numel(phi_range));
    DFx = zeros(Number_of_targets, numel(phi_range));
    DFy = zeros(Number_of_targets, numel(theta_range));
        
    for i=1:length(Targets_x)

        DFx(i, :) = Fx( (phi_range), (Targets_x(i)), Nx, Ny ) ;
        
        DFy(i, :) = Fy((theta_range), (Targets_y(i)), Nx, Ny ); 
    
    end


    %Компенсация: кросс-члены по целям
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
    

    if ro > 0
        %Попытка добавить ещё один множитель для компенсации искажений при ro > 0
        P_cross_x = zeros(1, numel(phi_range));
        P_cross_y = zeros(1, numel(theta_range));
        dCx = zeros(1, numel(phi_range));
        dCy = zeros(1, numel(theta_range));
        for i = 1:Number_of_targets
            P_cross_x = P_cross_x + abs(DFx(i,:)).^2;
            P_cross_y = P_cross_y + abs(DFy(i,:)).^2;
            for j = 1:Number_of_targets
                if j ~= i
                    Rij = ro * Ampl(i) * conj(Ampl(j));
                    dCx = dCx + ( ( Fy(Targets_y(i), Targets_y(j), Nx, Ny) ) / Ny) * Rij * conj(DFx(i,:)) .* DFx(j,:);
                    dCy = dCy + ( ( Fx(Targets_x(i), Targets_x(j), Nx, Ny) ) / Nx) * Rij * conj(DFy(i,:)) .* DFy(j,:);
                end
            end
        end
        P_cross = P_cross + P_cross_y.' * dCx + dCy.' * P_cross_x + dCy.' * dCx;
    end
end

