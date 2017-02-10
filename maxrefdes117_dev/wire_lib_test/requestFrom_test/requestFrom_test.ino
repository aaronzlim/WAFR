#include <Arduino.h>
#include <TinyWireM.h>
#include <USI_TWI_Master.h>
#include <Wire.h>
#include "my_max30102.h"


void setup() {
  Wire.begin();
  Serial.begin(115200);
  while(!Serial);
}

void loop() {
  Serial.print("\nFIFO WRITE PTR: ");
  Serial.print(read_reg(REG_FIFO_WR_PTR), HEX);
  Serial.print("\nFIFO OVERFLOW COUNTER: ");
  Serial.print(read_reg(REG_OVF_COUNTER), HEX);
  Serial.print("\nFIFO READ PTR: ");
  Serial.print(read_reg(REG_FIFO_RD_PTR), HEX);
  Serial.print("\nFIFO DATA: ");
  Serial.print(read_reg(REG_FIFO_DATA), HEX);

  delay(1000);
}

byte read_reg(byte addr) {

  Wire.beginTransmission(I2C_MAX30102_ADDR);
  Wire.write(addr);
  Wire.endTransmission(false);

  Wire.requestFrom(I2C_MAX30102_ADDR, 1);

  return Wire.read();
  
  
}

