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

// The I2C write address for the maxrefdes117 is given as 0xAE which is 174 in decimal
// however the last bit is the read/write bit, so the 7-bit address is 87 in decimal.

#define MAXREFDES_WRT_ADDR 87

void setup() {
	Wire.begin(MAXREFDES_WRT_ADDR); // Assigning address to the slave Flora
	Wire.onReceive(displayData); // Register a function to execute when the master sends data
	Serial.begin(115200); // Begin serial communication
	While(!Serial); // Wait for serial to start
	delay(1000);
}

void loop() {
	delay(100); // Do nothing
}

void displayData() {

	while(Wire.available()) {

		char c = Wire.read();
		Serial.print(c);

	}

}
