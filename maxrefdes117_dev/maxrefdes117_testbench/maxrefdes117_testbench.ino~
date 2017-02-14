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
#include "my_max30102.h"
//#include "algorithm.cpp"
#include <TinyWireM.h>
#include <USI_TWI_Master.h>
#include <Wire.h>
#include <SoftwareSerial.h>

// ------------------------ GLOBAL VARIABLE DECLARATIONS ------------------------
byte tx_status = 0;
/* NOTE ABOUT TX_STATUS FLAG
 * The Wire.endTransmission() function has 5 possible states
 * 0: success
 * 1: data too long to fit in transmit buffer
 * 2: received NACK on transmit of address
 * 3: received NACK on transmit of data
 * 4: other error
 *  
 * As a check use, tx_status = Wire.endTransmission();
 * then check to make sure it was successful */

const int32_t data_buffer_length = 100; //buffer length of 100 stores 4 seconds of samples running at 25sps
uint32_t ir_buffer[data_buffer_length]; // infrared LED sensor data
uint32_t red_buffer[data_buffer_length]; // red LED sensor data
int32_t spo2; // SPO2 value
int8_t spo2_valid; // indicator to show if the SPO2 calculation is valid
int32_t heart_rate; // heart rate value
int8_t hr_valid; // indicator to who if heart rate calculation is valid
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

uint32_t i; // incrementor
uint32_t j; // incrementor
uint8_t k; // incrementor

void loop() {
  
  for(i=0; i<data_buffer_length; i++) {
    
    while(digitalRead(10)==1); // Wait until the interrupt pin asserts

    // Read from max30102 FIFO
    max30102_read_fifo(i);
    //Serial.print("red=");
    //Serial.print(red_buffer[i], DEC);
    //Serial.print("\tir=");
    //Serial.print(ir_buffer[i], DEC);
    //Serial.print("\n");
  }

  for(i=0; i < 96; i++) {
    red_buffer[i] = (red_buffer[i] + red_buffer[i+1] + red_buffer[i+2] + red_buffer[i+3] + red_buffer[i+4])/5;
    Serial.println(red_buffer[i]);
    ir_buffer[i] = (ir_buffer[i] + ir_buffer[i+1] + ir_buffer[i+2] + ir_buffer[i+3] + ir_buffer[i+4])/5;
  }
  
  // CALCULATE HR AND SPO2
  //maxim_heart_rate_and_oxygen_saturation(ir_buffer, data_buffer_length, red_buffer, &spo2, &spo2_valid, &heart_rate, &hr_valid);

  // DISPLAY DATA
}

bool max30102_read_fifo(uint8_t idx) {

  byte temp[6]; // Holds one byte of data
  uint32_t red_sample = 0; // Holds the whole data word
  uint32_t ir_sample = 0;
  uint8_t fifo_read_ptr;
  uint8_t fifo_write_ptr;
  uint8_t num_samples = 0;
  
  // Clear the interrupt status registers
  max30102_clear_interrupt_status_regs();

  fifo_read_ptr = read_reg(REG_FIFO_RD_PTR); // Get current read ptr
  //Serial.print("\nSTART FIFO_RD_PTR: ");
  //Serial.print(fifo_read_ptr, HEX);
  //Serial.print("\n");
  
  fifo_write_ptr = read_reg(REG_FIFO_WR_PTR); // Get current write ptr
  //Serial.print("\nSTART FIFO_WR_PTR: ");
  //Serial.print(fifo_read_ptr, HEX);
  //Serial.print("\n");

  num_samples = fifo_write_ptr - fifo_read_ptr;

  if(!num_samples) {
    return false; // No data to read
  }
  
  // Read RED LED data from FIFO registers
  set_reg_ptr(REG_FIFO_DATA); // Point to fifo data register

  for(j=0; j<num_samples; j++) {
    Wire.requestFrom(I2C_MAX30102_ADDR, 6, true);
    k = 0;
    while(Wire.available()) {
      temp[k++] = Wire.read();
    }
    red_sample = ( (temp[0]<<16) | (temp[1]<<8) | temp[2] );
    red_sample &= 0x03FFFF;

    ir_sample = ( (temp[3]<<16) | (temp[4]<<8) | temp[5] );
    ir_sample &= 0x03FFFF;
  }

  //Serial.print("\nRED SAMPLE: ");
  red_buffer[idx] = red_sample;
  ir_buffer[idx] = ir_sample;

  //Serial.print("\nIR_SAMPLE: ");
  //Serial.print(ir_sample, DEC);
/*  
  Wire.requestFrom(I2C_MAX30102_ADDR, 1, true); // Request first byte of three
  temp = Wire.read();
  temp<<=16; // Shift by 2 bytes
  sample |= temp; // Store first byte

  fifo_read_ptr++;
  write_reg(REG_FIFO_RD_PTR, fifo_read_ptr); // Update read ptr

  set_reg_ptr(REG_FIFO_DATA); // point to fifo data
  
  Wire.requestFrom(I2C_MAX30102_ADDR, 1, true); // Request second byte of three
  temp = Wire.read(); // Store byte
  temp<<=8; // Shift by 1 byte
  sample |= temp; // Store second byte

  fifo_read_ptr++;
  write_reg(REG_FIFO_RD_PTR, fifo_read_ptr); // update read ptr

  set_reg_ptr(REG_FIFO_DATA); // point to fifo data
  
  Wire.requestFrom(I2C_MAX30102_ADDR, 1, true); // Request third byte of three
  temp = Wire.read(); // Store byte
  sample |= temp; // Store third byte

  fifo_read_ptr++;
  write_reg(REG_FIFO_RD_PTR, fifo_read_ptr); // upadate read ptr

  fifo_read_ptr = read_reg(REG_FIFO_RD_PTR); // Get current read ptr

  sample &= 0x03FFFF; // Mask bits [23:18]

  Serial.print("\nSAMPLE: ");
  Serial.print(sample, DEC);
  Serial.print("\n");

  Serial.print("\nEND FIFO_RD: ");
  Serial.print(fifo_read_ptr, HEX);
  Serial.print("\n");

  fifo_write_ptr = read_reg(REG_FIFO_WR_PTR); // get current write ptr

  Serial.print("\nEND FIFO_WRITE_PTR: ");
  Serial.print(fifo_write_ptr, HEX);
  Serial.print("\n");
  
/*
  // Read IR LED data from FIFO registers
  Wire.requestFrom(I2C_MAX30102_ADDR, 3, true);
  temp32 = Wire.read();
  Serial.print("\nIR: ");
  Serial.print(temp32, BIN);
  temp32<<=16;
  *ir_led += temp32;
  Wire.requestFrom(I2C_MAX30102_ADDR, 1, true);
  temp32 = Wire.read();
  temp32<<=8;
  *ir_led += temp32;
  Wire.requestFrom(I2C_MAX30102_ADDR, 1, true);
  temp32 = Wire.read();
  *ir_led += temp32;

  *red_led &= 0x03FFFF; // Mask MSB [23:18]
  *ir_led &= 0x03FFFF; // Mask MSB [23:18]
*/  

  return true;
}

void max30102_init(void) {
/* Inputs: None
 * Returns: tx_status (see line 20 for definition)
 * Function: Initialize a bunch of registers in max30102
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
  tx_status = Wire.endTransmission(true); // Full Stop

  Wire.beginTransmission(I2C_MAX30102_ADDR);
  Wire.write(REG_FIFO_CONFIG); // Set register pointer to 0x08
  Wire.write(0x4f); // Write to REG_FIFO_CONFIG (0x08)
                    // sample avg = 4, fifo rollover=false, fifo almost full = 17
  Wire.write(0x03); // Write to REG_MODE_CONFIG (0x09)
                    // 0x02 for Red only, 0x03 for SpO2 mode, 0x07 multimode LED
  Wire.write(0x27); // Write to REG_SPO2_CONFIG (0x0A)
                    // SPO2_ADC range = 4096nA, SPO2 sample rate (100 Hz), LED pulseWidth (411uS)
  tx_status = Wire.endTransmission(true); // Full Stop

  Wire.beginTransmission(I2C_MAX30102_ADDR);
  Wire.write(REG_LED1_PA); // Set register pointer to 0x0C
  Wire.write(0x24); // Write to REG_LED1_PA
                    //Choose value for ~ 7mA for LED1
  Wire.write(0x24); // Write to REG_LED2_PA
                    //Choose value for ~ 7mA for LED2
  tx_status = Wire.endTransmission(true); // Full Stop
  
  Wire.beginTransmission(I2C_MAX30102_ADDR);
  Wire.write(REG_PILOT_PA); // Set register pointer to 0x10
  Wire.write(0x7f); // Write to REG_PILOT_PA
                    // Choose value for ~ 25mA for Pilot LED
  tx_status = Wire.endTransmission(true);
}

byte max30102_clear_interrupt_status_regs(void) {
  /* Inputs: None
   * Returns: The value held in interrupt status 1 register (0x00)
   * Function: Reads/Clears interrupt status 1 register
   */
  Wire.beginTransmission(I2C_MAX30102_ADDR); // Get attention of max30102
  Wire.write(REG_INTR_STATUS_1); // Set register pointer to 0x00
  Wire.endTransmission(true); // Full Stop
  
  Wire.requestFrom(I2C_MAX30102_ADDR, 1, true); // Read from interrupt status regs
  Wire.requestFrom(I2C_MAX30102_ADDR, 1, true);
}

void max30102_reset(void) {
 /* Inputs: None
  * Returns: None
  * Function: Resets the MAX30102
  */
  Wire.beginTransmission(I2C_MAX30102_ADDR);
  Wire.write(REG_MODE_CONFIG); // Set register pointer to 0x09
  Wire.write(0x40); // Reset the max30102
  tx_status = Wire.endTransmission(true); // Full Stop
}

byte read_reg(byte reg) {

  byte data;
  
  Wire.beginTransmission(I2C_MAX30102_ADDR);
  Wire.write(reg);
  Wire.endTransmission(true);

  Wire.requestFrom(I2C_MAX30102_ADDR, 1, true);
  data = Wire.read();
  return data;
}

void write_reg(uint8_t addr, uint8_t data) {
  Wire.beginTransmission(I2C_MAX30102_ADDR);
  Wire.write(addr);
  Wire.write(data);
  Wire.endTransmission(true);
}

void set_reg_ptr(uint8_t addr) {
  Wire.beginTransmission(I2C_MAX30102_ADDR);
  Wire.write(addr);
  Wire.endTransmission(true);
}

