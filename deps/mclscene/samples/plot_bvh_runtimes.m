clear all;
close all,clc
set(0,'defaultLineLineWidth', 1.5)
hold on;

% Plot spatial and lienar bvh construction
    data1 = load('Projects/mclscene/build/bvh_spatial.txt');
    n_tris1 = data1(:,1) ./ 1000000;
    n_nodes1 = data1(:,2);
    time_s1 = data1(:,3);
    data2 = load('Projects/mclscene/build/bvh_linear.txt');
    n_tris2 = data2(:,1) ./ 1000000;
    n_nodes2 = data2(:,2);
    time_s2 = data2(:,3);
    
    plot(n_tris1,time_s1,'-',n_tris2,time_s2,'--');
    
    %set(gca,'xscale','log');
    %set(gca,'yscale','log');
    ylabel('Run Time (Seconds)','FontSize',12,'FontName','Helvetica','FontWeight','Bold')
    xlabel('Number of Triangles (Millions)','FontSize',12,'FontName','Helvetica','FontWeight','Bold')

legend('Object Median BVH','Linear BVH','Location','northwest');
hold off;

% resize and print
set(gcf, 'Position', [10 10 400 300]);
print('bvh_test','-depsc', '-r0');
print('bvh_test','-dpng', '-r0');

close all;
