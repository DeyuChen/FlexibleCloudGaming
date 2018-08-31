%print_format = '-depsc';
print_format = '-djpeg';

dirs = {'textured', 'non-textured'}

M = cell(2);

for d = 1:2

    root_path = char(dirs(d));

    M{d} = [];

    for i = 0:10:90
        for j = 0:1:15
            path = strcat(root_path, '/', int2str(i), '/client_frames/PSNR_delay_', int2str(j), '.txt');
            m = dlmread(path, ' ', 1);
            m = m(m(:, 1) <= 570 & m(:, 1) >= 30, :);
            m(isinf(m(:, 3)), 3) = 90;
            avg_PSNR = mean(m(:, 3));
            avg_SSIMR = mean(m(:, 4));
            avg_SSIMG = mean(m(:, 5));
            avg_SSIMB = mean(m(:, 6));
            M{d} = [M{d}; [i, j, avg_PSNR, avg_SSIMR, avg_SSIMG, avg_SSIMB]];
        end
        %path = strcat(root_path, '/', int2str(i), '/client_frames/PSNR_base.txt');
        %m = dlmread(path, ' ', 1);
        %m = m(m(:, 1) <= 570 & m(:, 1) >= 30, :)
        %avg_PSNR = mean(m(:, 3));
        %avg_SSIMR = mean(m(:, 4));
        %avg_SSIMG = mean(m(:, 5));
        %avg_SSIMB = mean(m(:, 6));
        %M = [M; [i, 32, avg_PSNR, avg_SSIMR, avg_SSIMG, avg_SSIMB]];
    end
    
    %{
    Mbase = [];
    
    for i = 0:10:90
        path = strcat(root_path, '/', int2str(i), '/client_frames/PSNR_base.txt');
        m = dlmread(path, ' ', 1);
        m = m(m(:, 1) <= 570 & m(:, 1) >= 30, :)
        avg_PSNR = mean(m(:, 3));
        avg_SSIMR = mean(m(:, 4));
        avg_SSIMG = mean(m(:, 5));
        avg_SSIMB = mean(m(:, 6));
        Mbase = [Mbase; [i, avg_PSNR, avg_SSIMR, avg_SSIMG, avg_SSIMB]];
    end
    %}

    %{
    for i = 0:10:90
        m = M{d}(M{d}(:, 1) == i, :);
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
    
    gscatter(M{d}(:, 2), M{d}(:, 3), M{d}(:, 1));
    set(gca, 'FontSize', 16);
    xlabel('Delay (# of frames)', 'FontSize', 16, 'FontWeight', 'bold');
    ylabel('Avg PSNR', 'FontSize', 18, 'FontWeight', 'bold');
    %hold on;
    %plot(x, y(1, :), x, y(2, :), x, y(3, :), x, y(4, :), x, y(5, :), x, y(6, :), x, y(7, :), x, y(8, :), x, y(9, :));
    %hold off;
    %for i = 1:10
    %    refline(0, Mbase(i, 2))
    %end
    print(strcat('figure/', root_path, '/psnr_all'), print_format);

    %{
    gscatter(M{d}(:, 2), M{d}(:, 4), M{d}(:, 1));
    set(gca, 'FontSize', 16);
    xlabel('Delay (# of frames)', 'FontSize', 16, 'FontWeight', 'bold');
    ylabel('Avg SSIM(R)', 'FontSize', 18, 'FontWeight', 'bold');
    %for i = 1:10
    %    refline(0, Mbase(i, 3))
    %end
    print(strcat('figure/', root_path, '/ssimr_all'), print_format);

    gscatter(M{d}(:, 2), M{d}(:, 5), M{d}(:, 1));
    set(gca, 'FontSize', 16);
    xlabel('Delay (# of frames)', 'FontSize', 16, 'FontWeight', 'bold');
    ylabel('Avg SSIM(G)', 'FontSize', 18, 'FontWeight', 'bold');
    %for i = 1:10
    %    refline(0, Mbase(i, 4))
    %end
    print(strcat('figure/', root_path, '/ssimg_all'), print_format);

    gscatter(M{d}(:, 2), M{d}(:, 6), M{d}(:, 1));
    set(gca, 'FontSize', 16);
    xlabel('Delay (# of frames)', 'FontSize', 16, 'FontWeight', 'bold');
    ylabel('Avg SSIM(B)', 'FontSize', 18, 'FontWeight', 'bold');
    %for i = 1:10
    %    refline(0, Mbase(i, 5))
    %end
    print(strcat('figure/', root_path, '/ssimb_all'), print_format);
    %}
    
    gscatter(M{d}(:, 2), (M{d}(:, 4) + M{d}(:, 5) + M{d}(:, 6)) / 3, M{d}(:, 1));
    set(gca, 'FontSize', 16);
    xlabel('Delay (# of frames)', 'FontSize', 16, 'FontWeight', 'bold')
    ylabel('Avg SSIM', 'FontSize', 18, 'FontWeight', 'bold');
    %for i = 1:10
    %    refline(0, Mbase(i, 5))
    %end
    print(strcat('figure/', root_path, '/ssim_all'), print_format);

end

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