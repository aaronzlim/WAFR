#include <Arduino.h>
#include <TinyWireM.h>
#include <USI_TWI_Master.h>
#include <Wire.h>
#include "my_max30102.h"

byte tx_status0 = 0;
byte tx_status1 = 0;
byte tx_status2 = 0;
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


byte before_msg;
byte after_msg;

void setup() {
  Wire.begin();
  Serial.begin(115200);
  while(!Serial);

  delay(2000);

  max30102_write_verify(REG_INTR_ENABLE_1, 0xc0);
  delay(500);
  check_msg();
  delay(1000);
  max30102_write_verify(REG_INTR_ENABLE_1, 0xd0);
  delay(500);
  check_msg();
  delay(1000);
}

void loop() {
  delay(1000); // Do nothing
}

void max30102_write_verify(byte addr, byte data) {

  before_msg = 0;
  after_msg = 0;

  Wire.beginTransmission(I2C_MAX30102_ADDR);
  Wire.write(addr); // Set register pointer to addr
  Wire.requestFrom(I2C_MAX30102_ADDR, 1, false); // Request 1 byte of data from max30102
  before_msg = Wire.read();
  tx_status0 = Wire.endTransmission(false); // Repeat Start
  Wire.write(addr); // Set register pointer to addr
  Wire.write(data); // Write data to register
  tx_status1 = Wire.endTransmission(false); // repeat start
  Wire.write(addr); // Set register pointer back to addr
  Wire.requestFrom(I2C_MAX30102_ADDR, 1); // Request 1 byte of data from max30102
  after_msg = Wire.read();
  tx_status2 = Wire.endTransmission(true); // Full stop

}

void check_msg() {

  if( (tx_status0!=0) || (tx_status1!=0) || (tx_status2!=0) ){
    Serial.print("\nTRANSMISSION ERROR:\n");
    Serial.print("tx_st0: ");
    Serial.print(tx_status0);
    Serial.print("\ntx_st1: ");
    Serial.print(tx_status1);
    Serial.print("\ntx_st2: ");
    Serial.print(tx_status2);
  } else {
    Serial.print("\nTRANSMISSION SUCCESS\n");
    Serial.print("BEFORE MESSAGE: ");
    Serial.print(before_msg, HEX);
    Serial.print("\nAFTER MESSAGE: ");
    Serial.print(after_msg, HEX);
  }
  
}


