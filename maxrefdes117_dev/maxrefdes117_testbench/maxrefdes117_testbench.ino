/*
* Aaron Lim
* Maxrefdes117 Testbench
* 2 February, 2017
*
* This is test code used to help interface the Maxrefdes117 pulse oximetry sensor with
* the Adafruit Flora.
*
*/

#include <Arduino.h>
#include "max30102.cpp"
#include <TinyWireM.h>
#include <USI_TWI_Master.h>
#include <Wire.h>
#include <SoftwareSerial.h>



void setup() {

  // Reset Max30102 by writing to REG_MODE_CONFIG 0x04
  
  Serial.begin(115200);
  Wire.begin();

  //pinMode(10, INPUT); // MAX30102 Interrupt Pin to D10 on Flora
  //delay(500);

  // Read REG_INTR_STATUS_1 &uch_dummy
}

void loop() {
  Wire.beginTransmission(8);
  Wire.write(4);
  Wire.endTransmission();
  delay(500);
}
