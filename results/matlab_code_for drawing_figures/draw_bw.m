%print_format = '-depsc';
print_format = '-djpeg';
lwidth = 2;

dirs = {'textured', 'non-textured'};
bws = [2000000, 4000000, 8000000, 16000000, 32000000, 64000000];

M = cell(2);

for d = 1:2

    root_path = char(dirs(d));

    M{d} = [];

    for i = 1:6
        path = strcat(root_path, '/bw/', int2str(bws(i)), '/client_frames/PSNR_delay_0.txt');
        m = dlmread(path, ' ', 1);
        m = m(m(:, 1) <= 570 & m(:, 1) >= 30, :);
        m(isinf(m(:, 3)), 3) = 90;
        avg_PSNR = mean(m(:, 3));
        avg_SSIMR = mean(m(:, 4));
        avg_SSIMG = mean(m(:, 5));
        avg_SSIMB = mean(m(:, 6));
        
        path = strcat(root_path, '/bw/', int2str(bws(i)), '/bw_log');
        m = dlmread(path, ' ', 0);
        avg_bw = mean(m(:, 1));
        
        M{d} = [M{d}; [bws(i), avg_PSNR, avg_SSIMR, avg_SSIMG, avg_SSIMB, avg_bw]];
    end
end

plot(M{1}(:, 1) / 1000000, M{1}(:, 6) / 1000, '--', M{2}(:, 1) / 1000000, M{2}(:, 6) / 1000, 'LineWidth', lwidth);
set(gca,'FontSize',16);
xlabel('H.264 bw allocation (Mbps)', 'FontSize', 16, 'FontWeight', 'bold');
xticks(bws / 1000000);
ylabel('Avg bw used (kB/frame)', 'FontSize', 18, 'FontWeight', 'bold');
legend('Textured', 'Non-textured', 'Location', 'SouthEast');
print('figure/bw_bw', print_format);

plot(M{1}(:, 1) / 1000000, M{1}(:, 2), '--', M{2}(:, 1) / 1000000, M{2}(:, 2), 'LineWidth', lwidth);
set(gca,'FontSize',16);
xlabel('H.264 bw allocation (Mbps)', 'FontSize', 16, 'FontWeight', 'bold');
xticks(bws / 1000000);
ylabel('Avg PSNR', 'FontSize', 18, 'FontWeight', 'bold');
legend('Textured', 'Non-textured', 'Location', 'SouthEast');
print('figure/bw_psnr', print_format);

plot(M{1}(:, 1) / 1000000, (M{1}(:, 3) + M{1}(:, 4) + M{1}(:, 5)) / 3, '--', M{2}(:, 1) / 1000000, (M{2}(:, 3) + M{2}(:, 4) + M{2}(:, 5)) / 3, 'LineWidth', lwidth);
set(gca,'FontSize',16);
xlabel('H.264 bw allocation (Mbps)', 'FontSize', 16, 'FontWeight', 'bold');
xticks(bws / 1000000);
ylabel('Avg SSIM', 'FontSize', 18, 'FontWeight', 'bold');
legend('Textured', 'Non-textured', 'Location', 'SouthEast');
print('figure/bw_ssim', print_format);
