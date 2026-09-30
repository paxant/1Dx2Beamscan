function [] = GraficFunc(theta_range,phi_range, Matrix, xlable_Name, ylabel_Name, title_Name)
% Визуализация спектра по угловым координатам
figure;
imagesc(theta_range, phi_range, Matrix);
colorbar;
xlabel(xlable_Name);
ylabel(ylabel_Name);
title(title_Name);
axis xy;
grid on;
colormap('jet');
end