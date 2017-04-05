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
#include "max30102_processing.h"
#include <SoftwareSerial.h>

uint32_t get_num_peaks(uint32_t *ir_buffer) {
  
  uint32_t ir_mean; // Holds DC mean of data
  uint32_t threshold = 0; // Minimum height of a peak
  uint32_t num_peaks = 0; // Count the number of peaks (heartbeats)
  int32_t peak_locs[MAX_NUM_PEAKS];
  uint16_t j;
  
  ir_mean = 0;
  for(j=0; j < BUFFER_SIZE; j++) {ir_mean += ir_buffer[j];}
  ir_mean = (ir_mean / BUFFER_SIZE);

  // Remove DC component from IR data
  for(j=0; j < BUFFER_SIZE; j++) tmp_ir[j] = (ir_buffer[j] < ir_mean ? 0 : (ir_buffer[j] - ir_mean));
  
  // Apply moving average filter
  for(j=0; j < ((BUFFER_SIZE - MVG_AVG_SIZE) + 1); j++) {
    tmp_ir[j] = ( tmp_ir[j] + tmp_ir[j+1] + tmp_ir[j+2] + tmp_ir[j+3] + tmp_ir[j+4]
                  + tmp_ir[j+5]) / MVG_AVG_SIZE;
  }

  // Calculate threshold 
  threshold = 0;
  for ( j=0 ; j<BUFFER_SIZE ;j++){threshold += tmp_ir[j];}
  threshold = (threshold / BUFFER_SIZE);
/*
  for(j= 0; j < 100; j++) {
    Serial.print(tmp_ir[j]);
    Serial.print(",");
    Serial.println(threshold);
  }
*/
  for(j = 0; j < MAX_NUM_PEAKS; j++) peak_locs[j] = 0;
  peaks_above_min_height(peak_locs, &num_peaks, tmp_ir, threshold, MAX_NUM_PEAKS);
  remove_close_peaks(tmp_ir, peak_locs, &num_peaks, MIN_PEAK_WIDTH);

  return num_peaks;
}

float get_spo2_ratio(uint32_t *red_buffer, uint32_t *ir_buffer) {

  uint16_t j; // Used for incrementing

  int32_t ir_dc = 0, red_dc = 0, ir_ac = 0, red_ac = 0;
  uint32_t num, den;
  
  // Load raw data again for SPO2 calculation
  for(j = 0; j < BUFFER_SIZE; j++) {
    tmp_ir[j] = ir_buffer[j];
    tmp_red[j] = red_buffer[j];
  }
  
  // Find IR and Red DC components
  for(j = 0; j < BUFFER_SIZE; j++) {
      red_dc += tmp_red[j];
      ir_dc += tmp_ir[j];
  }
  red_dc = (red_dc / BUFFER_SIZE);
  ir_dc = (ir_dc / BUFFER_SIZE);
  
  // Find IR and Red AC RMS components
  for(j = 0; j < BUFFER_SIZE; j++) { // Get pure AC
      tmp_ir[j] = (tmp_ir[j] - ir_dc);
      tmp_red[j] = (tmp_red[j] - red_dc);
  }
  // Implement root mean square
  for(j = 0; j < BUFFER_SIZE; j++) { // Sum of squares
      ir_ac += (tmp_ir[j] * tmp_ir[j]);
      red_ac += (tmp_red[j] * tmp_red[j]);
  }
  ir_ac = (ir_ac / BUFFER_SIZE); // Divided by window size
  red_ac = (red_ac / BUFFER_SIZE);
  
  ir_ac = (int32_t) sqrt(ir_ac);
  red_ac = (int32_t) sqrt(red_ac);
  
  num = (red_ac * ir_dc);
  den = (ir_ac * red_dc) >> 7; // Prepare for x100 to preserve decimal

  return num/den;
  
}

void peaks_above_min_height(int32_t *ir_locs, uint32_t *num_peaks, int32_t *tmp_ir, uint32_t threshold, uint32_t max_num_peaks) {
  /*
   * Find all peaks above threshold
   * 
   */

  uint32_t width = 0;
  uint32_t j = 1, k = 0;
  while(j < BUFFER_SIZE - 1 && k < max_num_peaks) {
      if ((tmp_ir[j] > tmp_ir[j-1]) && (tmp_ir[j] > threshold)) { // Found left edge of peak
          if(tmp_ir[j] == tmp_ir[j+1]) { // Found a flat section
              width = j;
              while((width < BUFFER_SIZE - 1) && (tmp_ir[width] == tmp_ir[width + 1])) { // Traverse flat peaks
                  width++;
              }
              if((width < BUFFER_SIZE - 1) && (tmp_ir[width] > tmp_ir[width+1])) { // Found a flat peak
                // FOUND A PEAK
                (*num_peaks)++;
                ir_locs[k++] = j;
                j = width; // skip over peak
              }
          }
          else if(tmp_ir[j] > tmp_ir[j+1]) { // Found a pointed peak
              // FOUND A PEAK
              (*num_peaks)++;
              ir_locs[k++] = j;
          }
      }
      j++;
  }
}

void remove_close_peaks(int32_t *tmp_ir, int32_t *ir_locs, uint32_t *num_peaks, uint32_t min_width) {
  /*
   * Remove peaks seperated by less than min_width
   * 
   */
  int16_t i, j, distance;

  // Sort peak locations descending (insertion sort)
  sort_descending(ir_locs, MAX_NUM_PEAKS);

  for(i = 0; i < MAX_NUM_PEAKS - 1; i++) {
      if(ir_locs[i] == 0) { break; } // Zero indicates no more peak locations
      for(j = i+1; j < MAX_NUM_PEAKS; j++) {
          if(ir_locs[j] == 0) { break; }
          distance = ir_locs[i] - ir_locs[j];
          if((distance <= min_width) && (distance != 0) && (ir_locs[j] != 0)) {  // Found close peaks
              if(tmp_ir[ir_locs[i]] > tmp_ir[ir_locs[j]]) { // remove smaller peak
                  ir_locs[j] = 0;
                  sort_descending(ir_locs, MAX_NUM_PEAKS);
              }
              else {
                  ir_locs[i] = 0;
                  sort_descending(ir_locs, MAX_NUM_PEAKS);
              }
              i--;
          }
          // else peaks are an acceptable distance apart
      }
  }
  
  *num_peaks = 0;
  
  for (i = 0; i < MAX_NUM_PEAKS; i++) {
      if(ir_locs[i] == 0) { break; }
      else { (*num_peaks)++; }
  }

  // Sort peak locations ascending (insertion sort)
  sort_ascending(ir_locs, MAX_NUM_PEAKS);
}

void sort_ascending(int32_t *arr, uint32_t len) {
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

void sort_descending(int32_t *arr, uint32_t len) {
    /*
     * arr - an array holding values
     * key - an array holding indexes for the values in arr
     * 
     */
    
    uint16_t i, j, temp;
    
    for (i = 0; i < len - 1; i++) {
        j = i + 1;
        temp = arr[j];
        while(j > 0 && temp > arr[j-1]) {
            arr[j] = arr[j-1];
            j--;
        }
        arr[j] = temp;
    }
}

