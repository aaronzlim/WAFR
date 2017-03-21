% Aaron Lim
% max30102_data_two_minutes.txt
% 28 February 2017


formatSpec = '%d';
% Read data
redFile = fopen('data_two_minutes_red.txt','r');
redData = fscanf(redFile, formatSpec);
fclose(redFile);

irFile = fopen('data_two_minutes_ir.txt','r');
irData = fscanf(irFile, formatSpec);
fclose(irFile);

sampleSize = length(redData);

% Calculate DC offset
redDC = 0;
irDC = 0;
for k = 1:sampleSize
    redDC = redDC + (redData(k)/sampleSize);
    irDC = irDC + (irData(k)/sampleSize);
end

%Remove DC offset
redAC = zeros(1,sampleSize);
irAC = zeros(1,sampleSize);
for k = 1:sampleSize
    redAC(k) = redData(k) - redDC;
    irAC(k) = irData(k) - irDC;
end

% Bad data points
redAC(1) = 0;
irAC(1) = 0;
irAC(2) = 0;

sampleSpace = 0:sampleSize-1;

red_fft = fft(redAC);
ir_fft = fft(irAC);

% Plot data in time domain
fig = figure();
subplot(2,1,1)
stem(sampleSpace, redAC);
title('Two Minutes of Red Data');
xlabel('Sample Number');
ylabel('ADC Value');
subplot(2,1,2);
stem(sampleSpace,irAC);
title('Two Minutes of IR Data');
xlabel('Sample Number');
ylabel('ADC Value');

% Plot fft data
fig_fft = figure();
subplot(2,1,1);
stem(sampleSpace,abs(red_fft));
title('Red FFT');
xlabel('Sample Number');
ylabel('Amplitude');
subplot(2,1,2);
stem(sampleSpace,abs(ir_fft));
title('TIR FFT');
xlabel('Sample Number');
ylabel('Amplitude');



