% Aaron Lim
% MAX30102 SPO2 Algorithm
% Raw Data
% 26 February 2017

format_spec = '%d'; % reading integers from file
data_length = 200; % samples per channel

mean_red = 0;
mean_ir = 0;



file_red = fopen('raw_data_red.txt','r');
data_red = fscanf(file_red, format_spec);
fclose(file_red);

file_ir = fopen('raw_data_ir.txt','r');
data_ir = fscanf(file_ir, format_spec);
fclose(file_ir);

fig = figure();
s1 = subplot(2,1,1);
stem(data_red);
title('Red Data');
xlabel('Samples');
ylabel('ADC Values');
s2 = subplot(2,1,2);
stem(data_ir);
title('IR Data');
xlabel('Samples');
ylabel('ADC Values');

% calculate means
for k = 1:data_length
    mean_red = (mean_red + data_red(k));
    mean_ir = (mean_ir + data_ir(k));
end
mean_red = (mean_red / data_length);
mean_ir = (mean_ir / data_length);

% remove means
for k = 1:data_length
    data_red(k) = ( data_red(k) - mean_red );
    data_ir(k) = ( data_ir(k) - mean_ir );
end

% apply a 4pt moving average
for k = 1:data_length-3
    data_red(k) = ( data_red(k) + data_red(k+1) + ...
                  data_red(k+2) + data_red(k+3) ) ...
                  / 4;
     data_ir(k) = ( data_ir(k) + data_ir(k+1) + ...
                  data_ir(k+2) + data_ir(k+3) ) ...
                  / 4;         
end

fig = figure();
s1 = subplot(2,1,1);
stem(data_red);
title('Red Data');
xlabel('Samples');
ylabel('ADC Values');
s2 = subplot(2,1,2);
stem(data_ir);
title('IR Data');
xlabel('Samples');
ylabel('ADC Values');
