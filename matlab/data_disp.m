% Author: Aaron Lim
% File: data_disp.m
% Project: WAFR
% 22 March 2017
% University of Rochester, ECE

% Arguments:
% fileName - String - name of file to open
% dataType - String - 'HR' or 'SPO2'

function data_disp(fileName, dataType)

    if ~(strcmp(dataType, 'HR') || strcmp(dataType, 'SPO2') || strcmp(dataType, 'HB'))
        error('dataType must be string HR, HB, or SPO2');
    end

    dataFile = fopen(fileName, 'r');
    data = fscanf(dataFile, '%d');
    fclose(dataFile);
    
    dataLength = length(data);
    samples = 0:dataLength-1;
    
    figure();
    plot(samples, data);
    title(strcat(dataType, ' Data'));
    xlabel('Sample Number');
    ylabel(dataType);
    if strcmp(dataType, 'HR')
        ylim([30, 200]);
    elseif strcmp(dataType, 'SPO2')
        ylim([60, 105]);
    end

end
