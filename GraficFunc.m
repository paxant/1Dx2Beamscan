function [] = GraficFunc(phi_range, theta_range, Matrix, xlable_Name, ylabel_Name, title_Name, PlotEnable)
    % Визуализация спектра по угловым координатам
    if PlotEnable == 1
        figure;
        imagesc(phi_range, theta_range, Matrix);
        colorbar;
        xlabel(xlable_Name);
        ylabel(ylabel_Name);
        title(title_Name);
        axis xy;
        grid on;
        colormap('jet');
    end
end