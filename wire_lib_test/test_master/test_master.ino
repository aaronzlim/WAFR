/*
* Aaron Lim
* test_master
* 2 February 2017
* Testing data transfer between two Arduino Flora over an I2C bus
*/

#include <Arduino.h>
#include <TinyWireM.h>
#include <USI_TWI_Master.h>
#include <Wire.h>

void setup() {

	Wire.begin();
	delay(3000); // Wait for slave device to get set up

}

void loop() {
	// Every 0.5s send the number 4 to the slave device
	Wire.beginTransmission(2); // Slave device should have address 0x02
	Wire.write(4); // 0b0000100
	Wire.endTransmission();
	delay(500);

}
