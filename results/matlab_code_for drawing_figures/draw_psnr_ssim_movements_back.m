%print_format = '-depsc';
print_format = '-djpeg';

models = {'castelia', 'paris', 'atlantis', 'city', 'town', 'nimbasa', 'oldtown'};
movements = {'forward', 'backward', 'left_shift', 'right_shift', 'left_turn', 'right_turn'};
dirs = {'textured'};

M = cell(length(movements));

for modelid = 1:length(models)
    mkdir(char(strcat('figure/', models(modelid))));
    for d = 1:length(dirs)
        mkdir(char(strcat('figure/', models(modelid), '/', dirs(d))));
        for move = 1:length(movements)

            root_path = char(strcat('models/', models(modelid), '/', dirs(d), '/movement/', movements(move)));

            M{move} = [];

            for i = 0:10:90
                path = strcat(root_path, '/', int2str(i), '/client_frames/PSNR_delay_0_buffer_1.txt');
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
                
                M{move} = [M{move}; [i, avg_PSNR, avg_SSIMR, avg_SSIMG, avg_SSIMB, avg_BW]];
            end
        end

        lwidth = 2;

        plot(M{1}(:, 1), M{1}(:, 2), M{2}(:, 1), M{2}(:, 2), '--', M{3}(:, 1), M{3}(:, 2), ':', M{4}(:, 1), M{4}(:, 2), '-+', M{5}(:, 1), M{5}(:, 2), '-o', M{6}(:, 1), M{6}(:, 2), '-x', 'LineWidth', lwidth);
        set(gca,'FontSize',16);
        xlabel('LOD', 'FontSize', 16, 'FontWeight', 'bold');
        ylabel('Average PSNR', 'FontSize', 18, 'FontWeight', 'bold');
        legend('forward', 'backward', 'left shift', 'right shift', 'left turn', 'right turn', 'Location', 'SouthEast');
        print(char(strcat('figure/', models(modelid), '/', dirs(d), '/psnr')), print_format);

        %{
        plot(M{1}(:, 1), M{1}(:, 3), '--', M{2}(:, 1), M{2}(:, 3), 'LineWidth', lwidth);
        set(gca,'FontSize',16);
        xlabel('Percentage of edges', 'FontSize', 16, 'FontWeight', 'bold');
        ylabel('Average SSIM(R)', 'FontSize', 18, 'FontWeight', 'bold');
        legend('forward', 'backward', 'left_shift', 'right_shift', 'left_turn', 'right_turn', 'Location', 'SouthEast');
        print(strcat('figure/', char(dirs(d)), '/ssimr'), print_format);

        plot(M{1}(:, 1), M{1}(:, 4), '--', M{2}(:, 1), M{2}(:, 4), 'LineWidth', lwidth);
        set(gca,'FontSize',16);
        xlabel('Percentage of edges', 'FontSize', 16, 'FontWeight', 'bold');
        ylabel('Average SSIM(G)', 'FontSize', 18, 'FontWeight', 'bold');
        legend('forward', 'backward', 'left_shift', 'right_shift', 'left_turn', 'right_turn', 'Location', 'SouthEast');
        print(strcat('figure/', char(dirs(d)), '/ssimg'), print_format);

        plot(M{1}(:, 1), M{1}(:, 5), '--', M{2}(:, 1), M{2}(:, 5), 'LineWidth', lwidth);
        set(gca,'FontSize',16);
        xlabel('Percentage of edges', 'FontSize', 16, 'FontWeight', 'bold');
        ylabel('Average SSIM(B)', 'FontSize', 18, 'FontWeight', 'bold');
        legend('forward', 'backward', 'left_shift', 'right_shift', 'left_turn', 'right_turn', 'Location', 'SouthEast');
        print(strcat('figure/', char(dirs(d)), '/ssimb'), print_format);
        %}

        plot(M{1}(:, 1), (M{1}(:, 3) + M{1}(:, 4) + M{1}(:, 5)) / 3, M{2}(:, 1), (M{2}(:, 3) + M{2}(:, 4) + M{2}(:, 5)) / 3, '--', M{3}(:, 1), (M{3}(:, 3) + M{3}(:, 4) + M{3}(:, 5)) / 3, ':', M{4}(:, 1), (M{4}(:, 3) + M{4}(:, 4) + M{4}(:, 5)) / 3, '-+', M{5}(:, 1), (M{5}(:, 3) + M{5}(:, 4) + M{5}(:, 5)) / 3, '-o', M{6}(:, 1), (M{6}(:, 3) + M{6}(:, 4) + M{6}(:, 5)) / 3, '-x', 'LineWidth', lwidth);
        set(gca,'FontSize',16);
        xlabel('LOD', 'FontSize', 16, 'FontWeight', 'bold');
        ylabel('Average SSIM', 'FontSize', 18, 'FontWeight', 'bold');
        legend('forward', 'backward', 'left shift', 'right shift', 'left turn', 'right turn', 'Location', 'SouthEast');
        print(char(strcat('figure/', models(modelid), '/', dirs(d), '/ssim')), print_format);
        
        plot(M{1}(:, 1), M{1}(:, 6), M{2}(:, 1), M{2}(:, 6), '--', M{3}(:, 1), M{3}(:, 6), ':', M{4}(:, 1), M{4}(:, 6), '-+', M{5}(:, 1), M{5}(:, 6), '-o', M{6}(:, 1), M{6}(:, 6), '-x', 'LineWidth', lwidth);
        set(gca,'FontSize',16);
        xlabel('LOD', 'FontSize', 16, 'FontWeight', 'bold');
        ylabel('Average BW Used', 'FontSize', 18, 'FontWeight', 'bold');
        legend('forward', 'backward', 'left shift', 'right shift', 'left turn', 'right turn', 'Location', 'SouthEast');
        print(char(strcat('figure/', models(modelid), '/', dirs(d), '/bw')), print_format);

        %{
        BW = cell(2);
        for d = 1:2
            root_path = char(strcat(dirs(d), '/movement/', movements(m)));
            for i = 0:10:100
                path = strcat(root_path, '/', int2str(i), '/bw_log');
                m = dlmread(path, ' ', 0);
                BW{d} = [BW{d}; [i, mean(m(30:end, 1))]];
            end
        end

        %plot(BW(:, 1), BW(:, 2) / 1000);
        plot(BW{1}(:, 1), BW{1}(:, 2) / 1000, '--', BW{2}(:, 1), BW{2}(:, 2) / 1000, 'LineWidth', lwidth);
        set(gca,'FontSize',16);
        xlabel('Percentage of edges', 'FontSize', 16, 'FontWeight', 'bold');
        ylabel('Average bw used (kB/frame)', 'FontSize', 18, 'FontWeight', 'bold');
        legend('Textured', 'Non-textured');
        print('figure/movement/', char(movements(move)), '/bandwidth', print_format);
        %}
    end
end