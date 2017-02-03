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
	Wire.begin(8); // Assigning address 0x08 to the slave Flora
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
