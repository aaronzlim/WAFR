% Aaron Lim
% digitalFilter.m
% WAFR
% 23 March 2017

% NOTE: Data should be line delimited integers

function digitalFilter(fileName)

    % Grab data from file
    dataFile = fopen(fileName, 'r');
    data = fscanf(dataFile, '%d');
    fclose(dataFile);
    
    dataLength = length(data);
    
    % If DC has not yet been removed, remove DC mean
    % and rectify the signal
    if max(data) > 300
        mean = sum(data) / dataLength;
        for k = 1:dataLength
            data(k) = data(k) - mean;
            if data(k) < 0
                data(k) = 0;
            end
        end
    end
    
    fData = zeros(size(data)); % Array to hold filtered data
    
    % Apply a FIR filter
    alpha = 0.5;
    
    for k = 2:dataLength
        fData(k) = data(k-1) + round(alpha*(data(k) - data(k-1)));
    end
    
    % Apply a moving average
    maData = zeros(size(data)); % Array to hold averaged data
    MA = 6;
    for k = 1:dataLength-(MA-1)
        maData(k) = round(sum(data(k:k+(MA-1))) / MA);
    end
    
    % Calculate means for each graph
    dataMean = zeros(size(data));
    fDataMean = zeros(size(data));
    maDataMean = zeros(size(data));
    
    for k = 1:dataLength
       dataMean(k) = sum(data(1:100))/(dataLength/2);
       fDataMean(k) = sum(fData(1:100))/(dataLength/2);
       maDataMean(k) = sum(maData(1:100))/(dataLength/2);
    end
    
    close all;
    
    figure();
    subplot(3,1,1);
    plot(data);
    hold on;
    plot(dataMean);
    title('Raw Data');
    xlabel('Samples');
    ylabel('Amplitude');
    subplot(3,1,2);
    plot(fData);
    hold on;
    plot(fDataMean);
    title('FIR Filtered Data (a = 0.5)');
    xlabel('Samples');
    ylabel('Amplitude');
    subplot(3,1,3);
    plot(maData);
    hold on;
    plot(maDataMean);
    title(strcat(int2str(MA),' Point Moving Averaged Data'));
    xlabel('Samples');
    ylabel('Amplitude');
    
end