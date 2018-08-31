print_format = '-depsc';
%print_format = '-djpeg';

models = {'oldtown', 'town', 'nimbasa'};
movements = {'forward', 'backward', 'left_shift', 'right_shift', 'left_turn', 'right_turn'};
dirs = {'textured'};

for d = 1:length(dirs)
    mkdir(char(strcat('figure/', dirs(d))));
    M = [];
    for modelid = 1:length(models)
        %for i = 0:10:90
        for i = 50
            for j = 0:1:15
                temp = [0, 0];
                for move = 1:length(movements)
                    if (i < 100)
                        root_path = char(strcat('models/', models(modelid), '/', dirs(d), '/movement/', movements(move)));
                        path = strcat(root_path, '/', int2str(i), '/client_frames/PSNR_delay_', int2str(j), '_buffer_1.txt');
                        m = dlmread(path, ' ', 1);
                        m = m(m(:, 1) >= 30 & m(:, 1) <= size(m, 1) - 30, :);
                        m(isinf(m(:, 3)), 3) = 90;
                        avg_PSNR = mean(m(:, 3));
                        avg_SSIMR = mean(m(:, 4));
                        avg_SSIMG = mean(m(:, 5));
                        avg_SSIMB = mean(m(:, 6));
                        avg_SSIM = (avg_SSIMR + avg_SSIMG + avg_SSIMB) / 3;
                    end

                    temp = temp + [avg_PSNR, avg_SSIM];
                end
                temp = temp / length(movements);
                M = [M; [i, j, modelid, temp]];
            end
        end

        lwidth = 2;
    end
    
    h = gscatter(M(:, 2), M(:, 4), M(:, 3), 'rgb', '.s^');
    set(gca, 'FontSize', 16);
    set(h(2), 'MarkerFaceColor', 'g');
    set(h(3), 'MarkerFaceColor', 'b');
    xlabel('Delay (# of frames)', 'FontSize', 16, 'FontWeight', 'bold');
    ylabel('Avg PSNR', 'FontSize', 18, 'FontWeight', 'bold');
    legend('Simple Model', 'Default Model', 'Complex Model', 'Location', 'NorthEast');
    print(char(strcat('figure/', dirs(d), '/psnr_all_models')), print_format);
    
    h = gscatter(M(:, 2), M(:, 5), M(:, 3), 'rgb', '.s^');
    set(gca, 'FontSize', 16);
    set(h(2), 'MarkerFaceColor', 'g');
    set(h(3), 'MarkerFaceColor', 'b');
    axisLimits = axis;
    axisLimits(3) = 0.89;
    axis(axisLimits);
    xlabel('Delay (# of frames)', 'FontSize', 16, 'FontWeight', 'bold');
    ylabel('Avg SSIM', 'FontSize', 18, 'FontWeight', 'bold');
    legend('Simple Model', 'Default Model', 'Complex Model', 'Location', 'SouthWest');
    print(char(strcat('figure/', dirs(d), '/ssim_all_models')), print_format);
    
    %{
    h = plot(M(:, 2), M(:, 4), 'linewidth', 2);
    set(gca, 'FontSize', 16);
    xlabel('Delay (# of frames)', 'FontSize', 16, 'FontWeight', 'bold');
    ylabel('Avg PSNR', 'FontSize', 18, 'FontWeight', 'bold');
    print(char(strcat('figure/', dirs(d), '/psnr_delay')), print_format);
    
    h = plot(M(:, 2), M(:, 5), 'linewidth', 2);
    set(gca, 'FontSize', 16);
    xlabel('Delay (# of frames)', 'FontSize', 16, 'FontWeight', 'bold');
    ylabel('Avg SSIM', 'FontSize', 18, 'FontWeight', 'bold');
    print(char(strcat('figure/', dirs(d), '/ssim_delay')), print_format);
    %}
    
    %{
    plot(M{1}(1:10, 1), M{1}(1:10, 2), '--', M{2}(1:10, 1), M{2}(1:10, 2), M{3}(1:10, 1), M{3}(1:10, 2), ':', 'LineWidth', lwidth);
    set(gca,'FontSize',16);
    xlabel('LOD', 'FontSize', 16, 'FontWeight', 'bold');
    ylabel('Average PSNR', 'FontSize', 18, 'FontWeight', 'bold');
    legend('Simple Model', 'Default Model', 'Complex Model', 'Location', 'NorthWest');
    print(char(strcat('figure/', dirs(d), '/psnr_models')), print_format);

    plot(M{1}(1:10, 1), (M{1}(1:10, 3) + M{1}(1:10, 4) + M{1}(1:10, 5)) / 3, '--', M{2}(1:10, 1), (M{2}(1:10, 3) + M{2}(1:10, 4) + M{2}(1:10, 5)) / 3, M{3}(1:10, 1), (M{3}(1:10, 3) + M{3}(1:10, 4) + M{3}(1:10, 5)) / 3, ':', 'LineWidth', lwidth);
    set(gca,'FontSize',16);
    xlabel('LOD', 'FontSize', 16, 'FontWeight', 'bold');
    ylabel('Average SSIM', 'FontSize', 18, 'FontWeight', 'bold');
    legend('Simple Model', 'Default Model', 'Complex Model', 'Location', 'SouthEast');
    print(char(strcat('figure/', dirs(d), '/ssim_models')), print_format);

    plot(M{1}(:, 1), M{1}(:, 6) / 1000, '--', M{2}(:, 1), M{2}(:, 6) / 1000, M{3}(:, 1), M{3}(:, 6) / 1000, ':', 'LineWidth', lwidth);
    set(gca,'FontSize',16);
    xlabel('LOD', 'FontSize', 16, 'FontWeight', 'bold');
    ylabel('Avg bw used (kB/frame)', 'FontSize', 18, 'FontWeight', 'bold');
    ylim([0 50])
    legend('Simple Model', 'Default Model', 'Complex Model', 'Location', 'SouthWest');
    print(char(strcat('figure/', dirs(d), '/bw_models')), print_format);
    %}
end