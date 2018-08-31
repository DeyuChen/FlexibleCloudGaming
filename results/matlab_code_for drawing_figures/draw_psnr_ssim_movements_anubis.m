%print_format = '-depsc';
print_format = '-djpeg';

models = {'town', 'anubis'};
movements = {'forward', 'backward', 'left_shift', 'right_shift', 'left_turn', 'right_turn'};
dirs = {'textured'};

M = cell(length(models));

for d = 1:length(dirs)
    mkdir(char(strcat('figure/', dirs(d))));
    for modelid = 1:length(models)
        M{modelid} = [];
        for i = 0:10:100
            temp = [0, 0, 0, 0, 0];
            for move = 1:length(movements)
                if (i < 100)
                    root_path = char(strcat('models/', models(modelid), '/', dirs(d), '/movement/', movements(move)));
                    path = strcat(root_path, '/', int2str(i), '/client_frames/PSNR_delay_0_buffer_1.txt');
                    m = dlmread(path, ' ', 1);
                    m = m(m(:, 1) >= 30 & m(:, 1) <= size(m, 1) - 30, :);
                    m(isinf(m(:, 3)), 3) = 90;
                    avg_PSNR = mean(m(:, 3));
                    avg_SSIMR = mean(m(:, 4));
                    avg_SSIMG = mean(m(:, 5));
                    avg_SSIMB = mean(m(:, 6));
                end
                
                path = strcat(root_path, '/', int2str(i), '/bw_log');
                m = dlmread(path, ' ', 0);
                m = m(m(:, 1) >= 30 & m(:, 1) <= size(m, 1) - 30, :);
                avg_BW = mean(m(:, 2));
                
                temp = temp + [avg_PSNR, avg_SSIMR, avg_SSIMG, avg_SSIMB, avg_BW];
            end
            temp = temp / length(movements);
            M{modelid} = [M{modelid}; [i, temp]];
        end

        lwidth = 2;
    end
    
    plot(M{1}(1:10, 1), M{1}(1:10, 2), '--', M{2}(1:10, 1), M{2}(1:10, 2), 'LineWidth', lwidth);
    set(gca,'FontSize',16);
    xlabel('LOD', 'FontSize', 16, 'FontWeight', 'bold');
    ylabel('Avg PSNR', 'FontSize', 18, 'FontWeight', 'bold');
    legend('Default Model', 'Empirical Model', 'Location', 'NorthWest');
    print(char(strcat('figure/', dirs(d), '/psnr_anubis')), print_format);

    plot(M{1}(1:10, 1), (M{1}(1:10, 3) + M{1}(1:10, 4) + M{1}(1:10, 5)) / 3, '--', M{2}(1:10, 1), (M{2}(1:10, 3) + M{2}(1:10, 4) + M{2}(1:10, 5)) / 3, 'LineWidth', lwidth);
    set(gca,'FontSize',16);
    xlabel('LOD', 'FontSize', 16, 'FontWeight', 'bold');
    ylabel('Avg SSIM', 'FontSize', 18, 'FontWeight', 'bold');
    legend('Default Model', 'Empirical Model', 'Location', 'SouthEast');
    print(char(strcat('figure/', dirs(d), '/ssim_anubis')), print_format);

    plot(M{1}(:, 1), M{1}(:, 6) / 1000, '--', M{2}(:, 1), M{2}(:, 6) / 1000, 'LineWidth', lwidth);
    set(gca,'FontSize',16);
    xlabel('LOD', 'FontSize', 16, 'FontWeight', 'bold');
    ylabel('Avg bw used (kB/frame)', 'FontSize', 18, 'FontWeight', 'bold');
    ylim([0 50])
    legend('Default Model', 'Empirical Model', 'Location', 'SouthWest');
    print(char(strcat('figure/', dirs(d), '/bw_anubis')), print_format);
end