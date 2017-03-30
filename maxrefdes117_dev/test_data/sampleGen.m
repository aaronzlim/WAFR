% Aaron Lim
% sampleGen.m
% WAFR
% 24 March 2017
% Generate a file with artificial sample data

function sampleGen()

data = [ 0   1   0   1   10  9   8   30  45  33  ...
         36  33  30  100 95  90  78  74  60  70   ...
         60  57  55  0   0   25  80  90  80  77  ...
         80  29  36  44  58  90  95  100 97  95  ...
         80  88  86  60  45  43  45  46  47  37  ...
         2   30  37  26  30  45  54  50  33  24  ...
         0   0   0   2   3   6   5   2   7   22  ...
         4   4   17  0   0   0   100 100 100 100 ...
         100 0   0   0   0   0   0   0   0   0   ...
         0   0   0   0   0   0   0   0   0   0   ];

for k = 1:100
    data(k) = data(k) + randi(20);
end

maData = zeros(size(data));
for k = 1:95
    maData(k) = round(sum(data(k:k+5)) / 6);
end
mean = 0;
for k = 1:100
    mean = mean + data(k);
end
mean = mean/100;

mean_arr = zeros(1,100);
for k = 1:100
    mean_arr(k) = mean;
end
     
figure();
subplot(2,1,1)
plot(data);
xlim([0,100]);
subplot(2,1,2);
plot(maData);
hold on;
plot(mean_arr);


timestamp = strcat('sample_75bpm_', datestr(now, 'dd_MM_ss'),'.txt');

%fileID = fopen(timestamp, 'w');
%fprintf(fileID, '%d\n', data);
%fclose(fileID);

end

