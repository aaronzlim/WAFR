% Aaron Lim
% compare_samples.m
% WAFR
% 22 March 2017
% University of Rochester, ECE


function compare_samples(wafrFileName, comparisonFileName, dataType)

    if(~dataType)
        error('dataType must be "HR" or "SPO2"');
    end

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
    
    time_multiplier = 3;
    
    x = time_multiplier .* x;
    err = abs(comparisonData - wafrData);
    avg_error = sum(err) / length(err);
    avg_error_arr = zeros(1,length(err));
    for k = 1:length(err)
        avg_error_arr(k) = avg_error;
    end
    
    % This section is for adding text to the error plot
    unit = '';
    title_txt = '';
    
    if(strcmp(dataType,'HR'))
        unit = ' bpm';
        title_txt = 'HR Comparison: WAFR vs Chest Strap';
    elseif(strcmp(dataType, 'SPO2'))
        unit = ' %';
        title_txt = 'SPO2 Comparison: WAFR vs AccUrate Pulse Oximeter';
    end
    
    avg_err_txt = ['Avg. Error: ', num2str(round(avg_error,1)), unit];
    
    % min and max vertical window values
    ymin = min(min(wafrData),min(comparisonData));
    ymax = max(max(wafrData),max(comparisonData));
    
    % Find index of max error
    max_err_idx = 0;
    max_err = max(err);
    for i = 1:length(err)
        if(err(i) == max_err)
            max_err_idx = i * time_multiplier;
            break;
        end
    end
    
    max_err_txt = ['\leftarrow Max Error ', num2str(max(err)), unit];
    
    % Plot the data
    
    figure();
    set(gcf, 'numbertitle', 'off', 'name', wafrFileName); %set figure title
    subplot(2,1,1);
    plot(x, wafrData);
    hold on;
    plot(x, comparisonData);
    hold off;
    title(title_txt);
    xlabel('Time (secs)');
    ylabel(dataType);
    ylim([ymin - 10, ymax + 10]);
    legend('WAFR', 'Comparison');
    legend('show');
    
    subplot(2,1,2);
    plot(x, err);
    hold on;
    text(max_err_idx, max_err, max_err_txt, 'HorizontalAlignment', 'left');
    plot(x, avg_error_arr);
    text(length(err)*3/2, avg_error+1, avg_err_txt, 'HorizontalAlignment', 'center');
    hold off;
    title('Error: Difference between WAFR and Comparison Device');
    xlabel('Time (secs)');
    ylabel('error (\Delta %)');
    ylim([0, max(err) + 5]);
    legend('Error', 'Average Error');
    legend('show');

end