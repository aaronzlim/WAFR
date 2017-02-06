/*
* Aaron Lim
* test_slave
* 2 February 2017
* Testing data transfer between two Arduino Flora over an I2C bus.
*/

#include <Arduino.h>

#if defined(ARDUINO_AVR_FLORA8)
  #include <TinyWireM.h>
  #include <USI_TWI_Master.h>
#endif

#include <Wire.h>

// The I2C write address of the Maxrefdes117 is given as 0xAE = 74 in decimal
// however the last bit is the read/write bit, so the 7-bit address is 87
// in decimal.

#define MAXREFDES_WRT_ADDR 87

void setup() {
	Wire.begin(); // Assigning 7 bit address 87 to the slave Flora
	delay(3000);
}

void loop() {

	// Send the number 4 to the slave device every 0.5s
	Wire.beginTransmission(MAXREFDES_WRT_ADDR);
	Wire.write(4); // Send the number 4
	Wire.endTransmission();
  
	delay(500);

}
