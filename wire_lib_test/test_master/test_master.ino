/*
* Aaron Lim
* test_slave
* 2 February 2017
* Testing data transfer between two Arduino Flora over an I2C bus.
*/

#include <Arduino.h>
#include <TinyWireM.h>
#include <USI_TWI_Master.h>
#include <Wire.h>

void setup() {
	Wire.begin(); // Assigning address 0x02 to the slave Flora
	delay(3000);
}

void loop() {
	
	// Send the number 4 to the slave device every 0.5s
	Wire.beginTransmission(8); // Slave device should have address 0x08 (first available)
	Wire.write(174); // Send the number 174
	Wire.endTransmission();

	delay(500);

}
