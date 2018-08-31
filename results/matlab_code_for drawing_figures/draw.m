filename = 'summary.txt';

M = csvread(filename, 1);

plot(M(:, 1), M(:, 4));
set(gca,'FontSize',16);
xlabel('Percentage of edges', 'FontSize', 16, 'FontWeight', 'bold');
ylabel('Average PSNR', 'FontSize', 18, 'FontWeight', 'bold');
print('figure/psnr', '-depsc');

plot(M(:, 1), M(:, 6) / 1000);
set(gca,'FontSize',16);
xlabel('Percentage of edges', 'FontSize', 16, 'FontWeight', 'bold');
ylabel('Average bandwidth (kB/frame)', 'FontSize', 18, 'FontWeight', 'bold');
print('figure/bandwidth', '-depsc');

M(1, 8) = 0;
plot(M(:, 1), M(:, 8) / 1000000);
set(gca,'FontSize',16);
xlabel('Percentage of edges', 'FontSize', 16, 'FontWeight', 'bold');
ylabel('Average Rendering Time (ms)', 'FontSize', 18, 'FontWeight', 'bold');
print('figure/rendering_time', '-depsc');