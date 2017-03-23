% Aaron Lim
% compare_samples.m
% WAFR
% 22 March 2017
% University of Rochester, ECE


function compare_samples(wafrFileName, comparisonFileName)

    close all;

    wafrFile = fopen(wafrFileName, 'r');
    wafrData = fscanf(wafrFile, '%d');
    fclose(wafrFile);
    
    comparisonFile = fopen(comparisonFileName, 'r');
    comparisonData = fscanf(comparisonFile, '%d');
    fclose(comparisonFile);

    wafrLength = length(wafrData);
    comparisonLength = length(comparisonData);
    difference = abs(wafrLength - comparisonLength);
    
    
    if wafrLength < comparisonLength
        
        x = 0:wafrLength-1;
        for k = comparisonLength:-1:comparisonLength - difference + 1
            comparisonData(k) = [];
        end
        
    elseif wafrLength > comparisonLength
        
        x = 0:comparisonLength-1;
        for k = wafrLength:-1:wafrLength - difference + 1
            wafrData(k) = [];
        end
        
    else
        x = 0:wafrLength-1;
    end
    
    x = 4.*x;
    disp(length(comparisonData));
    disp(length(wafrData));
    error = abs(comparisonData - wafrData) ./ comparisonData;
    error = 100 .* error;
    avg_error = sum(error) / length(error);
    avg_error_arr = zeros(1,length(error));
    for k = 1:length(error)
        avg_error_arr(k) = avg_error;
    end
    
    figure();
    subplot(2,1,1);
    plot(x, wafrData);
    hold on;
    plot(x, comparisonData);
    hold off;
    title('HR Comparison: WAFR vs AccuRate Pulse Oximeter');
    xlabel('Time (secs)');
    ylabel('Heart Rate');
    ylim([60,100]);
    legend('WAFR', 'Comparison');
    legend('show');
    
    subplot(2,1,2);
    plot(x, error);
    hold on;
    plot(x, avg_error_arr);
    hold off;
    title('Error: Difference between WAFR and Comparison Device');
    xlabel('Time (secs)');
    ylabel('error (%)');

end