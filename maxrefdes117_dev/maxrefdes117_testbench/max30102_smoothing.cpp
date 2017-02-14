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

uint32_t ir_mean; // Holds DC mean of IR data
uint8_t j; // Used for incrementing

void max30102_calc_hr_spo2(uint32_t *red_buffer, uint32_t *ir_buffer, int32_t *spo2, 
                           int8_t *spo2_valid, int32_t *heart_rate, int8_t *hr_valid) {

  // Using the IR data to calculate heart rate
  // Calculate the DC mean and subtract from the IR data
  ir_mean = 0;
  for(j=0; j < BUFFER_SIZE; j++) ir_mean += ir_buffer[j];
  ir_mean = ir_mean / BUFFER_SIZE;
  
  // Remove DC component from IR data
  for(j=0; j < BUFFER_SIZE; j++) tmp_ir[j] = (ir_buffer[j] - ir_mean);
  
  // Apply a 4 point moving average
  for(j=0; j < ((BUFFER_SIZE - MVG_AVG_SIZE) + 1); j++) {
    tmp_ir[j] = ( tmp_ir[j] + tmp_ir[j+1] + tmp_ir[j+2] + tmp_ir[j+3] ) / (int)4;
  }
  
  // DEBUGGING
  for(j=0; j < BUFFER_SIZE; j++) {
    ir_buffer[j] = tmp_ir[j];
  }
  
}

