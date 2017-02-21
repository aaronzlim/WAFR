/*
 * Author: Aaron Lim
 * Project: WAFR
 * Filename: max30102_smoothing.cpp
 * Description: Data smoothing algorithm for the max30102 pulse oximetry sensor
 * 
 * Revision History:
 * 2-14-17 Rev 1.00 Initial Release
 * 
 */

#include <Arduino.h>
#include "max30102_smoothing.h"
#include <SoftwareSerial.h>

void max30102_calc_hr_spo2(uint32_t *red_buffer, uint32_t *ir_buffer, int32_t *spo2, 
                           int8_t *spo2_valid, int32_t *heart_rate, int8_t *hr_valid) {

  uint32_t ir_mean; // Holds DC mean of IR data
  uint32_t threshold = 0; // Minimum height of a peak
  uint32_t num_peaks = 0; // Count the number of peaks (heartbeats)
  int32_t ir_locs[MAX_NUM_PEAKS];
  int32_t peak_interval_sum;
  uint16_t j, k; // Used for incrementing

  // Using the IR data to calculate heart rate
  // Calculate the DC mean and subtract from the IR data
  ir_mean = 0;
  for(j=0; j < BUFFER_SIZE; j++) {ir_mean += (ir_buffer[j] / BUFFER_SIZE);}

  // Remove DC component from IR data
  for(j=0; j < BUFFER_SIZE; j++) tmp_ir[j] = (ir_buffer[j] < ir_mean ? 0 : (ir_buffer[j] - ir_mean));
  
  // Apply a 4 point moving average
  for(j=0; j < ((BUFFER_SIZE - MVG_AVG_SIZE) + 1); j++) {
    tmp_ir[j] = ( tmp_ir[j] + tmp_ir[j+1] + tmp_ir[j+2] + tmp_ir[j+3] ) / (int)4;
  }

  // Calculate threshold 
  threshold = 0;
  for ( j=0 ; j<BUFFER_SIZE ;j++){threshold += tmp_ir[j];}
  threshold = (threshold / BUFFER_SIZE);

  for(j = 0; j < MAX_NUM_PEAKS; j++) ir_locs[j] = 0;
  find_peaks(ir_locs, &num_peaks, tmp_ir, threshold, MVG_AVG_SIZE, MAX_NUM_PEAKS );

  peak_interval_sum = 0;
  if(num_peaks >= 2) {
    for(j = 1; j < num_peaks; j++) {
      peak_interval_sum += ir_locs[j] - ir_locs[j-1];
    }
    peak_interval_sum = peak_interval_sum / (num_peaks - 1);
    *heart_rate = (int32_t) ( (FS*60) / peak_interval_sum );
    *hr_valid = 1;
  }
  else {
    *heart_rate = -999; // Not enough peaks to calculate HR
    *hr_valid = 0;
  }
/*
  // --------------------- SPO2 CALCULATION ------------------------
  int32_t ratio_average = 0, ratio_count = 0;
  int32_t ratio[RATIO_BUFFER_SIZE];
  int32_t ir_dc_max, red_dc_max, ir_dc_max_index, red_dc_max_index;
  int32_t ir_ac, red_ac;
  int32_t numerator, denominator;
  int32_t middle_index;
  uint32_t temp = 0;
  
  // Load raw data again for SPO2 calculation
  for(j = 0; j < BUFFER_SIZE; j++) {
    tmp_ir[j] = ir_buffer[j];
    tmp_red[j] = red_buffer[j];
  }

  // Find ir-red DC and ir-red AC maximums
  for(j = 0; j < RATIO_BUFFER_SIZE; j++) ratio[j] = 0; // Reset the ratio buffer
  for(j = 0; j < num_peaks; j++) {
    if(ir_locs[j] > BUFFER_SIZE) { // peak locations are out of range (data not valid)
      *spo2 = -999;
      *spo2_valid = 0;
      return;
    }
  }
Serial.print("\nMade it to line 85"); // BUG IN THIS LOOP AROUND LINE 91
  // Find max between two peak locations
  // and use the ratio between AC and DC components of ir and red for SPO2
  for(j = 0; j < num_peaks - 1; j++) {
    red_dc_max = -16777216; // Initialize to a very low number
    ir_dc_max = -16777216;
    if((ir_locs[j+1] - ir_locs[j]) > 3) {
      for(k = ir_locs[k]; k < ir_locs[k+1]; k++) {
        if(tmp_ir[k] > ir_dc_max) {ir_dc_max = tmp_ir[k]; ir_dc_max_index = k;}
        if(tmp_red[k] > red_dc_max) {red_dc_max = tmp_red[k]; red_dc_max_index = k;}
      }
      red_ac = (tmp_red[ir_locs[j+1]] - tmp_red[ir_locs[j]]) * (red_dc_max_index - ir_locs[j]); // red
      red_ac = tmp_red[ir_locs[j]] + (red_ac / (ir_locs[j+1] - ir_locs[j]));
      red_ac = tmp_red[red_dc_max_index] - red_ac;
      
      ir_ac = (tmp_ir[ir_locs[j+1]] - tmp_ir[ir_locs[j]]) * (ir_dc_max_index - ir_locs[j]); // ir
      ir_ac = tmp_ir[ir_locs[j]] + (ir_ac / (ir_locs[j+1] - ir_locs[j]));
      ir_ac = tmp_ir[ir_dc_max_index] - ir_ac;

      numerator = (red_ac * ir_dc_max) >> 7; // Prepare x100 to preserve floating value
      denominator = (ir_ac * red_dc_max) >> 7;
      if(denominator > 0 && ratio_count < 5 && numerator != 0) {
        //formular is ( n_y_ac *n_x_dc_max) / ( n_x_ac *n_y_dc_max)
        ratio[ratio_count] = (numerator*100) / denominator;
        ratio_count++;
      }
    }
  }
  Serial.print("Made it to line 112");
  // Choose median value since PPG signal may vary from beat to beat
  sort_ascending(ratio, ratio_count);
  middle_index = ratio_count / 2;

  if(middle_index > 1) {
    ratio_average = (ratio[middle_index -1] + ratio[middle_index]) / 2;
  }
  else {
    ratio_average = ratio[middle_index];
  }

  if(ratio_average > 2 && ratio_average < 184) {
    *spo2 = spo2_table[ratio_average]; // Consult look up table
    *spo2_valid = 1;
  }
  else {
    *spo2 = -999; // SPO2 ratio is out of range, bad data
    *spo2_valid = 0;
  }
*/  
  // DEBUGGING
  for(j=0; j < BUFFER_SIZE; j++) {
    ir_buffer[j] = tmp_ir[j];
  }
  
}

void find_peaks(int32_t *ir_locs, int32_t *num_peaks, int32_t *tmp_ir, uint32_t threshold, uint32_t min_width, uint32_t max_num_peaks) {
  /*
   *  PARAMS
   *  ------
   *  ir_locs - 
   *  num_peaks - counts the number of peaks found
   *  tmp_ir - The ir data buffer with smoothing applied
   *  threshold - minimum height of a peak
   *  min_width - minimum width of a peak
   *  max_num_peaks - maximum number of peaks that we can record
   */

  peaks_above_min_height(ir_locs, num_peaks, tmp_ir, &threshold, MAX_NUM_PEAKS);
  remove_close_peaks(tmp_ir, ir_locs, num_peaks, &min_width);
  (*num_peaks) = min( *num_peaks, max_num_peaks);   


}

void peaks_above_min_height(int32_t *ir_locs, int32_t *num_peaks, int32_t *tmp_ir, int32_t *threshold, uint32_t max_num_peaks) {
  /*
   * Find all peaks above threshold
   * 
   */

  uint32_t width = 0;
  uint16_t j = 1;
  while(j < (BUFFER_SIZE - 1)){

    if(tmp_ir[j] > *threshold && tmp_ir[j] > tmp_ir[j-1]) { // Find left edge of potential peak
      width = 1;
      while(j + width < BUFFER_SIZE && tmp_ir[j] == tmp_ir[j+width]) { // Find flat peaks
          width++;
      }
      if(tmp_ir[j] > tmp_ir[j+width] && (*num_peaks < MAX_NUM_PEAKS)) { // Find right edge of potential peak
        ir_locs[*num_peaks++] = j;
        j += width + 1;
      }
      else { j += width; }
    }
    else { j++; }
  }
}

void remove_close_peaks(uint32_t *tmp_ir, uint32_t *ir_locs, uint32_t *num_peaks, uint32_t min_width) {
  /*
   * Remove peaks seperated by less than a min_width
   * 
   */
  uint32_t temp, old_num_peaks, distance;
  int16_t i, j;

  // Sort peak locations descending (insertion sort)
  for(i = 1; i < *num_peaks; i++) {
    temp = ir_locs[i];
    for(j = i; j > 0 && tmp_ir[temp] > tmp_ir[ir_locs[j-1]]; j--) {
      ir_locs[j] = ir_locs[j-1];
    }
    ir_locs[j] = temp;
  }

  for(i = -1; i < *num_peaks; i++) {
    old_num_peaks = *num_peaks;
    *num_peaks = i + 1;
    for(j = i+1; j < old_num_peaks; j++) {
      distance = ir_locs[j] - ( i == -1 ? -1 : ir_locs[i] ); // Lag zero peak of autocorrelation is at index -1
      if(distance > min_width || distance < (-min_width)) {
        ir_locs[(*num_peaks)++] = ir_locs[j];
      }
    }
  }

  // Sort peak locations ascending (insertion sort)
  sort_ascending(ir_locs, *num_peaks);
}

void sort_ascending(uint32_t *arr, uint32_t len) {
  int i, j, tmp;
  for (i = 1; i < len; i++) {
    j = i;
    while (j > 0 && arr[j-1] > arr[j]) {
      tmp = arr[j];
      arr[j] = arr[j-1];
      arr[j-1] = tmp;
      j--;
    }
  }
}

