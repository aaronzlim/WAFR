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

/* NOTE ABOUT Wire.endTransmission()
 * The Wire.endTransmission() function has 5 possible states
 * 0: success
 * 1: data too long to fit in transmit buffer
 * 2: received NACK on transmit of address
 * 3: received NACK on transmit of data
 * 4: other error
 *  
 * As a check use, tx_status = Wire.endTransmission();
 * then check to make sure it was successful */

// ------------------------ GLOBAL VARIABLE DECLARATIONS ------------------------

const int32_t data_buffer_length = 100; //buffer length of 100 stores 4 seconds of samples running at 25sps
uint32_t red_buffer[data_buffer_length]; // red LED sensor data
uint32_t ir_buffer[data_buffer_length]; // infrared LED sensor data
int32_t spo2; // SPO2 value
int8_t spo2_valid; // indicator to show if the SPO2 calculation is valid
int32_t heart_rate; // heart rate value
int8_t hr_valid; // indicator if heart rate calculation is valid
uint32_t i; // incrementor
void *wire_obj_ptr; // Used to pass this Wire instance to the max30102 library

//---------------------------------------------------------------------------------

void setup() {

  Wire.begin(); // Join the bus
  max30102_reset(); // Reset the max30102
  Serial.begin(115200); // Serial communication will only be used for debugging
  while(!Serial);       // Wait for serial communication
  pinMode(10, INPUT); // MAX30102 interrupt output pin to D10 on Flora
  delay(1000);

  // Read/Clear REG_INTR_STATUS_1 (0x00)
  max30102_clear_interrupt_status_regs();
  
  // Initialize the max30102
  max30102_init();
}

void loop() {
  
  for(i=0; i<data_buffer_length; i++) {
    
    while(digitalRead(10)==1); // Wait until the interrupt pin asserts

    // Read from max30102 FIFO
    if(!max30102_read_fifo(red_buffer, ir_buffer, i)) {
      i--; // If there is nothing to read reset the incrementor and try again.
    }
  }

  
  // CALCULATE HR AND SPO2 (CURRENTLY THIS FUNCTION IS BEING DEBUGGED)
  max30102_calc_hr_spo2(red_buffer, ir_buffer, &spo2, &spo2_valid, &heart_rate, &hr_valid);

  // DISPLAY DATA (CURRENTLY DEBUGGING DATA)
  for(i=0; i < data_buffer_length - 3; i++) {
    Serial.println(ir_buffer[i]);
  }
}
