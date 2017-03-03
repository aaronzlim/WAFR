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
#include "max30102_smoothing.h"
#include <SoftwareSerial.h>
#include <TinyWireM.h>
#include <USI_TWI_Master.h>
#include <Wire.h>

// ------------------------ GLOBAL VARIABLE DECLARATIONS ------------------------

// DATA COLLECTION
const int32_t data_buffer_length = BUFFER_SIZE; //buffer length of 100 stores 4 seconds of samples running at 25sps
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

#define FLORA_INTR 10 // The interrupt line will go to pin 10 on the Flora

void setup() {

  Wire.begin(); // Join the bus
  max30102_reset(); // Reset the max30102
  Serial.begin(115200); // Serial communication will only be used for debugging
  while(!Serial);       // Wait for serial communication
  pinMode(FLORA_INTR, INPUT); // MAX30102 interrupt output pin to D10 on Flora
  delay(1000);

  // Read/Clear REG_INTR_STATUS_1 (0x00)
  max30102_clear_interrupt_status_regs();
  
  // Initialize the max30102
  max30102_init();
}

void loop() {
  // Put 5 num_peak samples in num_peaks_arr
  for(j = 0; j < INITIAL_SAMPLE_SIZE; j++) { // Collect 20s worth of peaks (heartbeats)
    // Collect 100 samples
    for(i=0; i<data_buffer_length; i++) {
    
      while(digitalRead(FLORA_INTR)==1); // Wait until the interrupt pin asserts

      // Read from max30102 FIFO
      if(!max30102_read_fifo(red_buffer, ir_buffer, i)) {
        i--; // If there is nothing to read decrement the incrementor and try again.
      }
    }
    num_peaks_arr[j] = get_num_peaks(ir_buffer);
    spo2_ratio_arr[j] = get_spo2_ratio(red_buffer, ir_buffer);
  }
  
  for(i = 0; i < INITIAL_SAMPLE_SIZE; i++) {
    total_num_peaks += num_peaks_arr[i];
  }

  // CALCULATE HR
  heart_rate = PEAKS_TO_HR(total_num_peaks);
  avg_hr = (heart_rate + avg_hr) / 2;

  // CALCULATE SPO2
  for(i = 0; i < INITIAL_SAMPLE_SIZE; i++) { avg_ratio += spo2_ratio_arr[i]; }
  avg_ratio = (avg_spo2 / (100 * INITIAL_SAMPLE_SIZE));
  spo2 = (uint32_t) ((-45.060 * avg_ratio * avg_ratio) + (30.354 * avg_ratio) + 94.845);
  avg_spo2 = (avg_spo2 + spo2) / 2;

  // NEED TO CHECK IF THESE ARE VALID!

  // DISPLAY DATA
  Serial.print("HEART RATE : ");
  Serial.println(avg_hr);
  Serial.print("SPO2 : ");
  Serial.println(avg_spo2);

} // END LOOP
