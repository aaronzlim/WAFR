/* 
* Author: Aaron Lim
* Project: WAFR
* File Name: max30102_wire_driver.cpp
* Release 14 February, 2017 V1.00 Initial Release
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

// REMEMBER TO USE Wire.begin() in your Arduino code

byte read_reg(byte reg) {
/*
* Inputs:   byte reg - Address of the register you want to read
* 
* Outputs:  byte data - The contents of register 'reg'
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

void write_reg(uint8_t addr, uint8_t data) {
/*
* Inputs:   uint8_t addr - Address to which you want to write
*           uint8_t data - One byte of data to write to the register
* 
* Outputs:  NONE
* 
* Function: Writes one byte of data to a register of the max30102
*/
  Wire.beginTransmission(I2C_MAX30102_ADDR);
  Wire.write(addr);
  Wire.write(data);
  Wire.endTransmission(true);
}

void set_reg_ptr(uint8_t addr) {
/*
* Inputs:   uint8_t addr - Address to point to
* 
* Outputs:  NONE
* 
* Function: Sets the max30102 register pointer to address 'addr'
*/
  Wire.beginTransmission(I2C_MAX30102_ADDR);
  Wire.write(addr);
  Wire.endTransmission(true);
}
