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
#include "algorithm.cpp"
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

// Dummy variable for general use
byte btemp = 0;

const int32_t data_buffer_length = 100; //buffer length of 100 stores 4 seconds of samples running at 25sps
uint32_t ir_buffer[data_buffer_length]; // infrared LED sensor data
uint32_t red_buffer[data_buffer_length]; // red LED sensor data
int32_t spo2; // SPO2 value
int8_t spo2_valid; // indicator to show if the SPO2 calculation is valid
int32_t heart_rate; // heart rate value
int8_t hr_valid; // indicator to who if heart rate calculation is valid
//---------------------------------------------------------------------------------

void setup() {

  Wire.begin();
  Serial.begin(115200); // Serial communication will only be used for debugging
  while(!Serial);       // Wait for serial communication

  // Initialize the max30102
  while(max30102_init()) {
    Serial.print("MAX30102 INIT ERROR: "); // Debugging
    Serial.print(tx_status);
    Serial.print("\n");
  }
  Serial.print("MAX30102 INIT SUCCESS\n");

  pinMode(10, INPUT); // MAX30102 interrupt output pin to D10 on Flora
  delay(500);

  // Read/Clear REG_INTR_STATUS_1 (0x00)
  max30102_read_interrupt_status_1();
  
}

void loop() {
  
  uint32_t i; // incrementor

  for(i=0; i<data_buffer_length; i++) {
    
    while(digitalRead(10)==1); // Wait until the interrupt pin asserts

    // Read from max30102 FIFO
    max30102_read_fifo(red_buffer, ir_buffer);
    
    Serial.print("red=");
    Serial.print(red_buffer[i], DEC);
    Serial.print("    ir=");
    Serial.print(ir_buffer[i], DEC);
  }
  
  // CALCULATE HR AND SPO2
  maxim_heart_rate_and_oxygen_saturation(ir_buffer, data_buffer_length, red_buffer, &spo2, &spo2_valid, &heart_rate, &hr_valid);

  // DISPLAY DATA
}

void max30102_read_fifo(uint32_t *red_led, uint32_t *ir_led) {

  uint32_t temp32;
  *ir_led = 0;
  *red_led = 0;
  
  // Clear the interrupt status registers
  Wire.beginTransmission(I2C_MAX30102_ADDR); // Talk to max30102
  Wire.write(REG_INTR_STATUS_1); // set register pointer to 0x00
  Wire.endTransmission(true); // Full Stop
  Wire.requestFrom(I2C_MAX30102_ADDR, 1, true); // read REG_INTR_STATUS_1, full stop
  Wire.requestFrom(I2C_MAX30102_ADDR, 1, true); // read REG_INTR_STATUS_2, full stop

  // Read RED LED data from FIFO registers
  Wire.beginTransmission(I2C_MAX30102_ADDR); // Talk to max30102
  Wire.write(REG_FIFO_DATA); // Set register pointer to 0x07
  Wire.endTransmission(true); // Full Stop
  
  Wire.requestFrom(I2C_MAX30102_ADDR, 1, true); // Request first byte of three
  temp32 = Wire.read(); // Store byte
  temp32<<=16; // Shift by 2 bytes
  *red_led += temp32; // Put byte in buffer
  Wire.requestFrom(I2C_MAX30102_ADDR, 1, true); // Request second byte of three
  temp32 = Wire.read(); // Store byte
  temp32<<=8; // Shift by 1 byte
  *red_led += temp32; // Put byte in buffer
  Wire.requestFrom(I2C_MAX30102_ADDR, 1, true); // Request third byte of three
  temp32 = Wire.read(); // Store byte
  *red_led += temp32; // Put byte in buffer

  // Read IR LED data from FIFO registers
  Wire.requestFrom(I2C_MAX30102_ADDR, 1, true);
  temp32 = Wire.read();
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
}

byte max30102_init(void) {
/* Inputs: None
 * Returns: tx_status (see line 20 for definition)
 * Function: Initialize a bunch of registers in max30102
 * NOTE: The register pointer in the max30102 autoincrements
 *       allowing us to burst write. The Wire library buffer
 *       is 32 bytes, so we can send a bunch of data at once.
 */
 
  Wire.beginTransmission(I2C_MAX30102_ADDR);
  Wire.write(REG_MODE_CONFIG); // Set register pointer to 0x09
  Wire.write(0x40); // Reset the max30102
  tx_status = Wire.endTransmission(true); // Full Stop
  if(tx_status) {
    return tx_status; // If it fails return and try again
  }

  delay(1000);

  Wire.beginTransmission(I2C_MAX30102_ADDR);
  Wire.write(REG_INTR_ENABLE_1); // Set register pointer to 0x02
  Wire.write(0xc0); // Write to REG_INTR_ENABLE_1 (0x02)
  Wire.write(0x00); // Write to REG_INTR_ENABLE_2 (0x03)
  Wire.write(0x00); // Write to REG_FIFO_WR_PTR (0x04)
  Wire.write(0x00); // Write to REG_OVF_COUNTER (0x05)
  Wire.write(0x00); // Write to REG_FIFO_RD_PTR (0x06)
  tx_status = Wire.endTransmission(true); // Full Stop
  if(tx_status) {
    return tx_status; // If it fails return and try again
  }

  Wire.beginTransmission(I2C_MAX30102_ADDR);
  Wire.write(REG_FIFO_CONFIG); // Set register pointer to 0x08
  Wire.write(0x4f); // Write to REG_FIFO_CONFIG (0x08)
                    // sample avg = 4, fifo rollover=false, fifo almost full = 17
  Wire.write(0x03); // Write to REG_MODE_CONFIG (0x09)
                    // 0x02 for Red only, 0x03 for SpO2 mode, 0x07 multimode LED
  Wire.write(0x27); // Write to REG_SPO2_CONFIG (0x0A)
                    // SPO2_ADC range = 4096nA, SPO2 sample rate (100 Hz), LED pulseWidth (411uS)
  tx_status = Wire.endTransmission(true); // Full Stop
  if(tx_status) {
    return tx_status; // If it fails return and try again
  }

  Wire.beginTransmission(I2C_MAX30102_ADDR);
  Wire.write(REG_LED1_PA); // Set register pointer to 0x0C
  Wire.write(0x24); // Write to REG_LED1_PA
                    //Choose value for ~ 7mA for LED1
  Wire.write(0x24); // Write to REG_LED2_PA
                    //Choose value for ~ 7mA for LED2
  tx_status = Wire.endTransmission(true); // Full Stop
  if(tx_status) {
    return tx_status; // If it fails return and try again
  }
  Wire.beginTransmission(I2C_MAX30102_ADDR);
  Wire.write(REG_PILOT_PA); // Set register pointer to 0x10
  Wire.write(0x7f); // Write to REG_PILOT_PA
                    // Choose value for ~ 25mA for Pilot LED
  tx_status = Wire.endTransmission(true);
  return tx_status;
}

byte max30102_read_interrupt_status_1(void) {
 /* Inputs: None
  * Returns: The value held in interrupt status 1 register (0x00)
  * Function: Reads/Clears interrupt status 1 register
  * 
  */
  Wire.beginTransmission(I2C_MAX30102_ADDR); // Get attention of max30102
  Wire.write(REG_INTR_STATUS_1); // Set register pointer to 0x00
  Wire.endTransmission(true); // Full Stop
  
  Wire.requestFrom(I2C_MAX30102_ADDR, 1, true); // Request 1 byte of data
  return Wire.read();
}



