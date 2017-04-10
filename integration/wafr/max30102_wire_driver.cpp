/* 
* Author: Aaron Lim
* Project: WAFR
* File Name: max30102_wire_driver.cpp
* Release: 2-14-2017 V1.0.0 BETA
* 
* This code is used to drive the max30102 pulse oximetry sensor using
* the Wire library for the Arduino platform.
*
* The project specifically uses the Adafruit Flora (Atmega32u4)
*/

#include <Arduino.h>
#include "max30102_wire_driver.h"
#include <TinyWireM.h>
#include <USI_TWI_Master.h>
#include <Wire.h>
#include <SoftwareSerial.h>

// Remember to call Wire.begin in your Arduino sketch
// The Wire object only uses class variables, so even though
// this instance will be different from the one in the Arduino
// sketch, they will be accessing the same data. So there is
// no need to pass the Arduino sketch instance into this library.

byte max30102_read_reg(byte reg) {
/*
* Inputs:   byte reg - Address of the register you want to read
* 
* Returns:  byte data - The contents of register 'reg'
* 
* Function: Reads a register from the max30102
*/

  uint8_t data;
  
  Wire.beginTransmission(I2C_MAX30102_ADDR);
  Wire.write(reg);
  Wire.endTransmission(true);

  Wire.requestFrom(I2C_MAX30102_ADDR, 1, true);
  data = Wire.read();
  return data;
}

void max30102_write_reg(uint8_t addr, uint8_t data) {
/*
* Inputs:   uint8_t addr - Address to which you want to write
*           uint8_t data - One byte of data to write to the register
* 
* Returns:  NONE
* 
* Function: Writes one byte of data to a register of the max30102
*/
  Wire.beginTransmission(I2C_MAX30102_ADDR);
  Wire.write(addr);
  Wire.write(data);
  Wire.endTransmission(true);
}

void max30102_set_reg_ptr(uint8_t addr) {
/*
* Inputs:   uint8_t addr - Address to point to
* 
* Returns:  NONE
* 
* Function: Sets the max30102 register pointer to address 'addr'
*/
  Wire.beginTransmission(I2C_MAX30102_ADDR);
  Wire.write(addr);
  Wire.endTransmission(true);
}

bool max30102_read_fifo(uint32_t *red_buffer, uint32_t *ir_buffer, uint8_t idx) {
/* Inputs: uint32_t* red_buffer - array of red LED data
 *		   uint32_t* ir_buffer - array of IR LED data
 *		   uint8_t idx - index of red_buffer and ir_buffer to place the FIFO buffer sample
 *
 *
 */
  uint32_t temp[6]; // Array to hold red and ir samples (each 3 bytes)
  uint32_t red_sample = 0; // Holds the whole sample (18 bits)
  uint32_t ir_sample = 0;
  uint8_t fifo_read_ptr;
  uint8_t fifo_write_ptr;
  uint8_t num_samples = 0;
  
  max30102_clear_interrupt_status_regs(); // Clear the max30102 interrupt status registers
  fifo_read_ptr = max30102_read_reg(REG_FIFO_RD_PTR); // Get current read ptr
  fifo_write_ptr = max30102_read_reg(REG_FIFO_WR_PTR); // Get current write ptr
  num_samples = fifo_write_ptr - fifo_read_ptr; // Calculate number of available samples in max30102 FIFO

  if(!num_samples) {
    return false; // No data to read
  }

  if(fifo_write_ptr == 0) {
    // For some reason when the buffer overflows the read pointer resets to register 4 instead of 0.
    // These lines make sure the next iteration puts the read pointer at 0 and the write pointer at 1.
    max30102_write_reg(REG_FIFO_RD_PTR, 0x1B);
    max30102_write_reg(REG_FIFO_WR_PTR, 0x1C);
  }
  
  // Read LED data from FIFO registers
  max30102_set_reg_ptr(REG_FIFO_DATA); // Point to fifo data register (0x07)

  for(uint8_t j=0; j<num_samples; j++) {
    Wire.requestFrom(I2C_MAX30102_ADDR, 6, true);
    uint8_t k = 0;
    while(Wire.available()) {
      temp[k++] = Wire.read();
    }
    red_sample = ( (temp[0]<<16) | (temp[1]<<8) | temp[2] );
    red_sample &= 0x03FFFF;

    ir_sample = ( (temp[3]<<16) | (temp[4]<<8) | temp[5] );
    ir_sample &= 0x03FFFF;
  }

  if(fifo_read_ptr != 0 && fifo_write_ptr != 0) {
    // Was getting weird data when the fifo rolled over from 31 to 0
    // choosing to ignore the first and last pieces of data
    red_buffer[idx] = red_sample;
    ir_buffer[idx] = ir_sample;
  }

  return true;
}

void max30102_init(void) {
/* Inputs: NONE
 * 
 * Returns: NONE
 * 
 * Function: Initialize a bunch of registers in max30102
 * 
 * NOTE: The register pointer in the max30102 autoincrements
 *       allowing us to burst write. The Wire library buffer
 *       is 32 bytes, so we can send a bunch of data at once.
 */
 
  Wire.beginTransmission(I2C_MAX30102_ADDR);
  Wire.write(REG_INTR_ENABLE_1); // Set register pointer to 0x02
  Wire.write(0xc0); // Write to REG_INTR_ENABLE_1 (0x02)
  Wire.write(0x00); // Write to REG_INTR_ENABLE_2 (0x03)
  Wire.write(0x00); // Write to REG_FIFO_WR_PTR (0x04)
  Wire.write(0x00); // Write to REG_OVF_COUNTER (0x05)
  Wire.write(0x00); // Write to REG_FIFO_RD_PTR (0x06)
  Wire.endTransmission(true); // Full Stop

  Wire.beginTransmission(I2C_MAX30102_ADDR);
  Wire.write(REG_FIFO_CONFIG); // Set register pointer to 0x08
  Wire.write(0x4f); // Write to REG_FIFO_CONFIG (0x08)
                    // sample avg = 4, fifo rollover=false, fifo almost full = 17
  Wire.write(0x03); // Write to REG_MODE_CONFIG (0x09)
                    // 0x02 for Red only, 0x03 for SpO2 mode, 0x07 multimode LED
  Wire.write(0x27); // Write to REG_SPO2_CONFIG (0x0A)
                    // SPO2_ADC range = 4096nA, SPO2 sample rate (100 Hz), LED pulseWidth (411uS)
  Wire.endTransmission(true); // Full Stop

  Wire.beginTransmission(I2C_MAX30102_ADDR);
  Wire.write(REG_LED1_PA); // Set register pointer to 0x0C
  Wire.write(0x24); // Write to REG_LED1_PA
                    //Choose value for ~ 7mA for LED1
  Wire.write(0x24); // Write to REG_LED2_PA
                    //Choose value for ~ 7mA for LED2
  Wire.endTransmission(true); // Full Stop
  
  Wire.beginTransmission(I2C_MAX30102_ADDR);
  Wire.write(REG_PILOT_PA); // Set register pointer to 0x10
  Wire.write(0x7f); // Write to REG_PILOT_PA
                    // Choose value for ~ 25mA for Pilot LED
  Wire.endTransmission(true);
}

void max30102_clear_interrupt_status_regs(void) {
 /* Inputs: None
  * 
  * Returns: The value held in interrupt status 1 register (0x00)
  * 
  * Function: Reads/Clears interrupt status registers
  */
  
  Wire.beginTransmission(I2C_MAX30102_ADDR); // Get attention of max30102
  Wire.write(REG_INTR_STATUS_1); // Set register pointer to 0x00
  Wire.endTransmission(true); // Full Stop
  
  Wire.requestFrom(I2C_MAX30102_ADDR, 1, true); // Read from interrupt status regs
  Wire.requestFrom(I2C_MAX30102_ADDR, 1, true);
}

void max30102_reset(void) {
/* Inputs: None
 *
 * Returns: None
 *
 * Function: Resets the max30102 optical sensor
 */
  Wire.beginTransmission(I2C_MAX30102_ADDR); // Get attention of max30102
  Wire.write(REG_MODE_CONFIG); // Set register pointer to 0x09
  Wire.write(0x40); // Reset max30102 by setting bit 6 and clearing all others
  Wire.endTransmission(true);
}

