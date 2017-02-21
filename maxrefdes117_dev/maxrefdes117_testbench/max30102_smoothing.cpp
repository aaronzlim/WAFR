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
  uint16_t j; // Used for incrementing

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

  find_peaks(ir_locs, &num_peaks, tmp_ir, threshold, MVG_AVG_SIZE, MAX_NUM_PEAKS );

  // CONTINUE HERE
  
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
  for(i = 1; i < *num_peaks; i++) {
    temp = ir_locs[i];
    for(j = i; j > 0 && temp < ir_locs[j-1]; j--) {
        ir_locs[j] = ir_locs[j-1];
    }
    ir_locs[j] = temp;
  }
}






