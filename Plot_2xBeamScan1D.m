function [] = Plot_2xBeamScan1D(BEAMscan_x,BEAMscan_y, phi_range, theta_range)

    figure;
    % BEAMscan_x
    subplot(2,1,1);
    plot(phi_range, 10*log10(abs(BEAMscan_x) ./ max(abs(BEAMscan_x))), ...
        'b', 'LineWidth', 1.5);
    grid on;
    xlabel('Угол \phi, ^\circ');
    ylabel('Уровень, dB');
    title('BEAMscan_x');
    xlim([-30 30]);
    ylim([-60 0]);
    
    % BEAMscan_y
    subplot(2,1,2);
    plot(theta_range, 10*log10(abs(BEAMscan_y) ./ max(abs(BEAMscan_y))), ...
        'r', 'LineWidth', 1.5);
    grid on;
    xlabel('Угол \theta, ^\circ');
    ylabel('Уровень, dB');
    title('BEAMscan_y');
    xlim([-30 30]);
    ylim([-60 0]);
end

