print_format = '-depsc';
%print_format = '-djpeg';

dirs = {'textured', 'non-textured'};
%dirs = {'textured'};
delays = [5, 10, 15];
lods = [0, 40, 80];
lods_label = {'low', 'medium', 'high'}

M = cell(2);

for d = 1:length(dirs)

    root_path = char(dirs(d));

    M{d} = [];

    for i = 1:length(lods)
        for j = 1:length(delays)
            for k = 1:10
                path = strcat(root_path, '/', int2str(lods(i)), '/client_frames/PSNR_delay_', int2str(delays(j)), '_buffer_', int2str(k), '.txt');
                m = dlmread(path, ' ', 1);
                m = m(m(:, 1) <= 570 & m(:, 1) >= 30, :);
                m(isinf(m(:, 3)), 3) = 90;
                avg_PSNR = mean(m(:, 3));
                avg_SSIMR = mean(m(:, 4));
                avg_SSIMG = mean(m(:, 5));
                avg_SSIMB = mean(m(:, 6));
                avg_SSIM = (avg_SSIMR + avg_SSIMG + avg_SSIMB) / 3;
                avg_FILLED = mean(m(:, 7));
                M{d} = [M{d}; [lods(i), delays(j), k, avg_PSNR, avg_SSIMR, avg_SSIMG, avg_SSIMB, avg_SSIM, avg_FILLED]];
            end
        end
    end

    %{
    for i = 1:length(lods)
        m = M{d}(M{d}(:, 1) == lods(i), :);
        %mm = Mbase(Mbase(:, 1) == i, :);

        plot(m(:, 2), m(:, 3));
        %refline(0, mm(:, 2));
        set(gca, 'FontSize', 16);
        xlabel('Delay (# of frames)', 'FontSize', 16, 'FontWeight', 'bold');
        ylabel('Avg PSNR', 'FontSize', 18, 'FontWeight', 'bold');
        print(strcat('figure/', root_path, '/psnr_', int2str(i)), print_format);

        plot(m(:, 2), m(:, 4));
        %refline(0, mm(:, 3));
        set(gca, 'FontSize', 16);
        xlabel('Delay (# of frames)', 'FontSize', 16, 'FontWeight', 'bold');
        ylabel('Avg SSIM(R)', 'FontSize', 18, 'FontWeight', 'bold');
        print(strcat('figure/', root_path, '/ssimr_', int2str(i)), print_format);

        plot(m(:, 2), m(:, 5));
        %refline(0, mm(:, 4));
        set(gca, 'FontSize', 16);
        xlabel('Delay (# of frames)', 'FontSize', 16, 'FontWeight', 'bold');
        ylabel('Avg SSIM(G)', 'FontSize', 18, 'FontWeight', 'bold');
        print(strcat('figure/', root_path, '/ssimg_', int2str(i)), print_format);

        plot(m(:, 2), m(:, 6));
        %refline(0, mm(:, 5));
        set(gca, 'FontSize', 16);
        xlabel('Delay (# of frames)', 'FontSize', 16, 'FontWeight', 'bold');
        ylabel('Avg SSIM(B)', 'FontSize', 18, 'FontWeight', 'bold');
        print(strcat('figure/', root_path, '/ssimb_', int2str(i)), print_format);
    end
    %}

    %x = [0, 30];
    %y = [Mbase(:, 2), Mbase(:, 2)];
    
    for i = 1:length(lods)
        if i == 3
            legend_location = 'NorthEast';
        else
            legend_location = 'SouthEast';
        end
        
        m = M{d}(M{d}(:, 1) == lods(i), :);

        h = gscatter(m(:, 3), m(:, 4), m(:, 2), [], 'o^s');
        set(h(1), 'MarkerFaceColor', 'r');
        set(h(2), 'MarkerFaceColor', 'g');
        set(h(3), 'MarkerFaceColor', 'b');
        h = legend('Location', legend_location);
        v = get(h, 'title');
        set(v, 'string', 'Latency');
        set(gca, 'FontSize', 16);
        xlabel('Buffer size (# of frames)', 'FontSize', 16, 'FontWeight', 'bold');
        ylabel('Avg PSNR', 'FontSize', 18, 'FontWeight', 'bold');
        print(strcat('figure/', root_path, '/psnr_', lods_label{i}), print_format);

        h = gscatter(m(:, 3), m(:, 8), m(:, 2), [], 'o^s');
        set(h(1), 'MarkerFaceColor', 'r');
        set(h(2), 'MarkerFaceColor', 'g');
        set(h(3), 'MarkerFaceColor', 'b');
        h = legend('Location', legend_location);
        v = get(h, 'title');
        set(v, 'string', 'Latency');
        set(gca, 'FontSize', 16);
        xlabel('Buffer size (# of frames)', 'FontSize', 16, 'FontWeight', 'bold');
        ylabel('Avg SSIM', 'FontSize', 18, 'FontWeight', 'bold');
        print(strcat('figure/', root_path, '/ssim_', lods_label{i}), print_format);

        h = gscatter(m(:, 3), m(:, 9), m(:, 2), [], 'o^s');
        set(h(1), 'MarkerFaceColor', 'r');
        set(h(2), 'MarkerFaceColor', 'g');
        set(h(3), 'MarkerFaceColor', 'b');
        h = legend();
        v = get(h, 'title');
        set(v, 'string', 'Latency');
        set(gca, 'FontSize', 16);
        xlabel('Buffer size (# of frames)', 'FontSize', 16, 'FontWeight', 'bold');
        ylabel('Avg # of pixels recovered', 'FontSize', 18, 'FontWeight', 'bold');
        print(strcat('figure/', root_path, '/pixels_', lods_label{i}), print_format);
    end

    %{
    gscatter(M{d}(:, 2), M{d}(:, 4), M{d}(:, 1));
    set(gca, 'FontSize', 16);
    xlabel('Delay (# of frames)', 'FontSize', 16, 'FontWeight', 'bold');
    ylabel('Avg SSIM(R)', 'FontSize', 18, 'FontWeight', 'bold');
    print(strcat('figure/', root_path, '/ssimr_all'), print_format);

    gscatter(M{d}(:, 2), M{d}(:, 5), M{d}(:, 1));
    set(gca, 'FontSize', 16);
    xlabel('Delay (# of frames)', 'FontSize', 16, 'FontWeight', 'bold');
    ylabel('Avg SSIM(G)', 'FontSize', 18, 'FontWeight', 'bold');
    print(strcat('figure/', root_path, '/ssimg_all'), print_format);

    gscatter(M{d}(:, 2), M{d}(:, 6), M{d}(:, 1));
    set(gca, 'FontSize', 16);
    xlabel('Delay (# of frames)', 'FontSize', 16, 'FontWeight', 'bold');
    ylabel('Avg SSIM(B)', 'FontSize', 18, 'FontWeight', 'bold');
    print(strcat('figure/', root_path, '/ssimb_all'), print_format);
    
    gscatter(M{d}(:, 2), (M{d}(:, 4) + M{d}(:, 5) + M{d}(:, 6)) / 3, M{d}(:, 1));
    set(gca, 'FontSize', 16);
    xlabel('Delay (# of frames)', 'FontSize', 16, 'FontWeight', 'bold')
    ylabel('Avg SSIM', 'FontSize', 18, 'FontWeight', 'bold');
    print(strcat('figure/', root_path, '/ssim_all'), print_format);
    %}

end

%{
lwidth = 2;

M1 = M{1}(M{1}(:, 2) == 0, :);
M2 = M{2}(M{2}(:, 2) == 0, :);

%plot(m(:, 1), m(:, 3));
plot(M1(:, 1), M1(:, 3), '--', M2(:, 1), M2(:, 3), 'LineWidth', lwidth);
set(gca,'FontSize',16);
xlabel('Percentage of edges', 'FontSize', 16, 'FontWeight', 'bold');
ylabel('Average PSNR', 'FontSize', 18, 'FontWeight', 'bold');
legend('Textured', 'Non-textured', 'Location', 'SouthEast');
print('figure/psnr', print_format);

%plot(m(:, 1), m(:, 4));
plot(M1(:, 1), M1(:, 4), '--', M2(:, 1), M2(:, 4), 'LineWidth', lwidth);
set(gca,'FontSize',16);
xlabel('Percentage of edges', 'FontSize', 16, 'FontWeight', 'bold');
ylabel('Average SSIM(R)', 'FontSize', 18, 'FontWeight', 'bold');
legend('Textured', 'Non-textured', 'Location', 'SouthEast');
print('figure/ssimr', print_format);

%plot(m(:, 1), m(:, 5));
plot(M1(:, 1), M1(:, 5), '--', M2(:, 1), M2(:, 5), 'LineWidth', lwidth);
set(gca,'FontSize',16);
xlabel('Percentage of edges', 'FontSize', 16, 'FontWeight', 'bold');
ylabel('Average SSIM(G)', 'FontSize', 18, 'FontWeight', 'bold');
legend('Textured', 'Non-textured', 'Location', 'SouthEast');
print('figure/ssimg', print_format);

%plot(m(:, 1), m(:, 6));
plot(M1(:, 1), M1(:, 6), '--', M2(:, 1), M2(:, 6), 'LineWidth', lwidth);
set(gca,'FontSize',16);
xlabel('Percentage of edges', 'FontSize', 16, 'FontWeight', 'bold');
ylabel('Average SSIM(B)', 'FontSize', 18, 'FontWeight', 'bold');
legend('Textured', 'Non-textured', 'Location', 'SouthEast');
print('figure/ssimb', print_format);
    
%plot(m(:, 1), (m(:, 4) + m(:, 5) + m(:, 6)) / 3);
plot(M1(:, 1), (M1(:, 4) + M1(:, 5) + M1(:, 6)) / 3, '--', M2(:, 1), (M2(:, 4) + M2(:, 5) + M2(:, 6)) / 3, 'LineWidth', lwidth);
set(gca,'FontSize',16);
xlabel('Percentage of edges', 'FontSize', 16, 'FontWeight', 'bold');
ylabel('Average SSIM', 'FontSize', 18, 'FontWeight', 'bold');
legend('Textured', 'Non-textured', 'Location', 'SouthEast');
print('figure/ssim', print_format);

BW = cell(2);
for d = 1:2
    root_path = char(dirs(d));
    for i = 0:10:100
        path = strcat(root_path, '/', int2str(i), '/bw_log');
        m = dlmread(path, ' ', 0);
        BW{d} = [BW{d}; [i, mean(m(30:570, 1))]];
    end
end

%plot(BW(:, 1), BW(:, 2) / 1000);
plot(BW{1}(:, 1), BW{1}(:, 2) / 1000, '--', BW{2}(:, 1), BW{2}(:, 2) / 1000, 'LineWidth', lwidth);
set(gca,'FontSize',16);
xlabel('Percentage of edges', 'FontSize', 16, 'FontWeight', 'bold');
ylabel('Average bw used (kB/frame)', 'FontSize', 18, 'FontWeight', 'bold');
legend('Textured', 'Non-textured');
print('figure/bandwidth', print_format);
%}