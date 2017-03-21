/*
* Author: Aaron Lim
* Project: WAFR
* File Name: maxrefdes117_testbench.ino
* Release: 2 February, 2017 V1.00 Initial Release
*
* This is test code used to help interface the Maxrefdes117 pulse oximetry sensor with
* the Adafruit Flora using the Wire library.
*/

#include <Arduino.h>
#include "max30102_wire_driver.h"
#include "max30102_processing.h"
#include <SoftwareSerial.h>
#include <TinyWireM.h>
#include <USI_TWI_Master.h>
#include <Wire.h>

// ------------------------ GLOBAL VARIABLE DECLARATIONS ------------------------

// MAX30102 DATA COLLECTION
const int32_t data_buffer_length = BUFFER_SIZE; // Defined in max30102_smoothing.h
uint32_t red_buffer[data_buffer_length]; // red LED sensor data
uint32_t ir_buffer[data_buffer_length]; // infrared LED sensor data

// HR CALCULATION
uint32_t num_peaks_arr[INITIAL_SAMPLE_SIZE];
uint32_t total_num_peaks = 0;
int32_t heart_rate, avg_hr = 75; // heart rate value
bool hr_valid; // indicator if heart rate calculation is valid

// SPO2 CALCULATION
uint32_t spo2_ratio_arr[INITIAL_SAMPLE_SIZE];
float avg_ratio = 0;
uint32_t avg_spo2 = 99;
int32_t spo2; // SPO2 value
bool spo2_valid; // indicator to show if the SPO2 calculation is valid

uint32_t i, j; // incrementor

//---------------------------------------------------------------------------------

#define MAX30102_INTR 10 // The interrupt line will go to pin 10 on the Flora

void setup() {

  Wire.begin(); // Join the bus
  max30102_reset(); // Reset the max30102
  Serial.begin(115200); // Serial communication will only be used for debugging
  while(!Serial);       // Wait for serial communication
  pinMode(MAX30102_INTR, INPUT); // MAX30102 interrupt output pin to D10 on Flora
  delay(1000);

  // Read/Clear REG_INTR_STATUS_1 (0x00)
  max30102_clear_interrupt_status_regs();
  
  // Initialize the max30102
  max30102_init();

  max30102_first_buffer_load(); // Load 20s of data into buffers
}

void loop() {

  max30102_calc_hr_spo2();  // get hr and spo2 from initial data at setup
  
  // NEED TO CHECK IF THESE ARE VALID!

  // DISPLAY DATA
  //Serial.println("------------------------------");
  //Serial.print("HEART RATE : ");
  Serial.println(avg_hr);
  //Serial.print("SPO2 : ");
  //Serial.println(avg_spo2);
  //Serial.println("------------------------------");

  max30102_get_new_sample(); // get a new sample

} // END LOOP

void max30102_get_new_sample() {

  int m;
  // Collect 100 new samples (4 seconds)
  for(m = 0; m < data_buffer_length; m++) {
    while(digitalRead(MAX30102_INTR) == 1); // Wait until the interrupt pin asserts
    if(!max30102_read_fifo(red_buffer, ir_buffer, m)) {
      m--; // If there is nothing to read decrement and try again
    }
  }

  for(m = 0; m < INITIAL_SAMPLE_SIZE - 1; m++) {
    num_peaks_arr[m] = num_peaks_arr[m+1];
    spo2_ratio_arr[m] = spo2_ratio_arr[m+1];
  }
  num_peaks_arr[INITIAL_SAMPLE_SIZE - 1] = get_num_peaks(ir_buffer);
  spo2_ratio_arr[INITIAL_SAMPLE_SIZE - 1] = get_spo2_ratio(red_buffer, ir_buffer);
}

void max30102_calc_hr_spo2() {

  total_num_peaks = 0;
  for(i = 0; i < INITIAL_SAMPLE_SIZE; i++) {
    total_num_peaks += num_peaks_arr[i];
  }

  // CALCULATE HR
  heart_rate = PEAKS_TO_HR(total_num_peaks);
  if(heart_rate < 225 && heart_rate > 30) {
      avg_hr = (heart_rate + avg_hr) / 2;
      hr_valid = 1;
  }
  else { hr_valid = 0; }

  // CALCULATE SPO2
  for(i = 0; i < INITIAL_SAMPLE_SIZE; i++) { avg_ratio += spo2_ratio_arr[i]; }
  avg_ratio = (avg_ratio / (100 * INITIAL_SAMPLE_SIZE));
  spo2 = (uint32_t) ((-55.426 * avg_ratio * avg_ratio) + (50.129 * avg_ratio) + 89.612);
  if(spo2 <= 100 && spo2 >= 80) {
      avg_spo2 = (avg_spo2 + spo2) / 2;
      spo2_valid = 1;
  }
  else {
    spo2_valid = 0;
  }
  
}

void max30102_first_buffer_load() {

  int m, n;
  
  // Put 5 num_peak samples in num_peaks_arr
  for(m = 0; m < INITIAL_SAMPLE_SIZE; m++) { // Collect 20s worth of peaks (heartbeats)
    // Collect 100 samples
    for(n=0; n<data_buffer_length; n++) {
    
      while(digitalRead(MAX30102_INTR)==1); // Wait until the interrupt pin asserts

      // Read from max30102 FIFO
      if(!max30102_read_fifo(red_buffer, ir_buffer, n)) {
        n--; // If there is nothing to read decrement the incrementor and try again.
      }
    }
    num_peaks_arr[m] = get_num_peaks(ir_buffer);
    spo2_ratio_arr[m] = get_spo2_ratio(red_buffer, ir_buffer);
  }
  
}

