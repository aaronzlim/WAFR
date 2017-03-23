% Aaron Lim
% MAX30102 HR algorithm
% Sample Data
% 22 February 2017

fileID = fopen('max30102_sample_data.txt','r');
formatSpec = '%d';
data = fscanf(fileID, formatSpec);
fclose(fileID);

mean = 0;
for i = 1:200
   mean = mean + data(i);
end
mean = (mean / 200);
disp('MEAN: ');
display(mean);
mean_arr = zeros(1,200);
for i = 1:200
   mean_arr(i) = mean; 
end

samples = 0:length(data) - 1;

fig = figure;
plot(samples, data);
hold on;
plot(samples,mean_arr);
xlabel('Samples');
ylabel('ADC Value');
title('MAX30102 Sample Data');