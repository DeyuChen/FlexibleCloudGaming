print_format = '-depsc';
%print_format = '-djpeg';

models = {'town', 'anubis', 'oldtown', 'nimbasa'};
movements = {'forward', 'backward', 'left_shift', 'right_shift', 'left_turn', 'right_turn'};
%dirs = {'textured', 'non-textured'};
dirs = {'textured'};
lod = {'0', '30', '60', '90'};

M = cell(length(dirs));

for modelid = 1:length(models)
    for d = 1:length(lod)
        M{d} = [];
        for i = 0:1:5
            temp = [0, 0, 0, 0, 0];
            for move = 1:length(movements)
                root_path = char(strcat('loss/', models(modelid), '/LOD', lod(d), '/textured/movement/', movements(move)));
                path = strcat(root_path, '/', int2str(i), '/client_frames/PSNR_delay_0_buffer_1.txt');
                disp(path);
                m = dlmread(path, ' ', 1);
                m = m(m(:, 1) >= 30 & m(:, 1) <= size(m, 1) - 30, :);
                m(isinf(m(:, 3)), 3) = 90;
                avg_PSNR = mean(m(:, 3));
                avg_SSIMR = mean(m(:, 4));
                avg_SSIMG = mean(m(:, 5));
                avg_SSIMB = mean(m(:, 6));
                
                path = strcat(root_path, '/', int2str(i), '/bw_log');
                m = dlmread(path, ' ', 0);
                m = m(m(:, 1) >= 30 & m(:, 1) <= size(m, 1) - 30, :);
                avg_BW = mean(m(:, 2));
                
                %path = strcat(root_path, '/', int2str(i), '/timing_log');
                %m = dlmread(path, ' ', 0);
                %m = m(30:size(m, 1)-30, 1);
                %avg_time = mean(m(:, 1));
                
                %temp = temp + [avg_PSNR, avg_SSIMR, avg_SSIMG, avg_SSIMB, avg_BW, avg_time];
                temp = temp + [avg_PSNR, avg_SSIMR, avg_SSIMG, avg_SSIMB, avg_BW];
            end
            temp = temp / length(movements);
            M{d} = [M{d}; [i, temp]];
        end

        lwidth = 2;
    end
    
    plot(M{1}(:, 1), M{1}(:, 2), '--', M{2}(:, 1), M{2}(:, 2), M{3}(:, 1), M{3}(:, 2), ':', M{4}(:, 1), M{4}(:, 2), '-.*', 'LineWidth', lwidth);
    set(gca,'FontSize',16);
    ylim([0, 90]);
    xlabel('Loss rate (%)', 'FontSize', 16, 'FontWeight', 'bold');
    ylabel('Avg PSNR', 'FontSize', 18, 'FontWeight', 'bold');
    legend('LOD 0', 'LOD 30', 'LOD 60', 'LOD 90', 'Location', 'SouthWest');
    print(char(strcat('figure/psnr_loss_', models(modelid))), print_format);

    plot(M{1}(:, 1), (M{1}(:, 3) + M{1}(:, 4) + M{1}(:, 5)) / 3, '--', M{2}(:, 1), (M{2}(:, 3) + M{2}(:, 4) + M{2}(:, 5)) / 3, M{3}(:, 1), (M{3}(:, 3) + M{3}(:, 4) + M{3}(:, 5)) / 3, ':', M{4}(:, 1), (M{4}(:, 3) + M{4}(:, 4) + M{4}(:, 5)) / 3, '-.*', 'LineWidth', lwidth);
    set(gca,'FontSize',16);
    xlabel('Loss rate (%)', 'FontSize', 16, 'FontWeight', 'bold');
    ylabel('Avg SSIM', 'FontSize', 18, 'FontWeight', 'bold');
    legend('LOD 0', 'LOD 30', 'LOD 60', 'LOD 90', 'Location', 'SouthWest');
    print(char(strcat('figure/ssim_loss_', models(modelid))), print_format);

    plot(M{1}(:, 1), M{1}(:, 6) / 1000, '--', M{2}(:, 1), M{2}(:, 6) / 1000, M{3}(:, 1), M{3}(:, 6) / 1000, ':', M{4}(:, 1), M{4}(:, 6) / 1000, '-.*', 'LineWidth', lwidth);
    set(gca,'FontSize',16);
    xlabel('Loss rate (%)', 'FontSize', 16, 'FontWeight', 'bold');
    ylabel('Avg bw used (kB/frame)', 'FontSize', 18, 'FontWeight', 'bold');
    legend('LOD 0', 'LOD 30', 'LOD 60', 'LOD 90', 'Location', 'SouthWest');
    print(char(strcat('figure/bandwidth_loss_', models(modelid))), print_format);
    
    %M{1}(1, 7) = 0;
    %M{2}(1, 7) = 0;
    
    %plot(M{1}(:, 1), M{1}(:, 7) / 1000000, '--', M{2}(:, 1), M{2}(:, 7) / 1000000, 'LineWidth', lwidth);
    %set(gca,'FontSize',16);
    %xlabel('LOD', 'FontSize', 16, 'FontWeight', 'bold');
    %ylabel('Avg rendering time (ms)', 'FontSize', 18, 'FontWeight', 'bold');
    %legend('textured', 'non-textured', 'Location', 'SouthEast');
    %print(char(strcat('figure/rendering_time')), print_format);
end