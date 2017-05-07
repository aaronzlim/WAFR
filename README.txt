WEARABLE DATA ACQUISITION FOR FIRST RESPONDERS (WAFR)

Project Team
------------
Aaron Lim
Justin Fraumeni
Jen Becerra
Christian Riedelsheimer

Organization
------------
University of Rochester
Department of Electrical and Computer Engineering
Senior Design Project

Revision History
----------------
10-17-2016 Revision 1.00 Initial

DESCRIPTION
-----------
First responders often find themselves in dangerous situations. It would be helpful to be able to track a few biological signals in order to assess whether a first responder is in a state of distress. We believe that pulse, blood oxygen saturation, and continued movement are good variables to determine such a condition. We propose a small, low power, and simple wearable device that will be able to measure these three factors and transmit them to a third party platform.

Technology Used
---------------
- Adafruit Flora (atmega32u4)
- Maxrefdes117 optical sensor system (Maxim Integrated)
  - MAX30102
- Adafruit Flora Bluefruit (Bluetooth Low Energy w/ UART)
- I2C interface (TWI)
  - Arduino Wire library
- MMA8451 Triple Axis Accelerometer
