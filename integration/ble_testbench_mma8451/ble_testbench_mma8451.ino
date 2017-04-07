/*
 * Authors: Aaron Lim
 *          Justin Fraumeni
 *          Jen Becerra
 *          Christian Reidelsheimer
 *          
 * Project: WAFR - Wearable Alert for First Responders
 * 
 * Description: Wearable heart rate and spo2 monitor with motion detection
 * 
 * Organization: University of Rochester
 *               Department of Electrical & Computer Engineering
 *               Senior Design              
 * 
 * Date: 30 March 2017
 * 
 * Version: 1.0.0 BETA
 * 
 */

#include <Arduino.h>
#include <TinyWireM.h>
#include <USI_TWI_Master.h>
#include <Wire.h>

#include <Adafruit_MMA8451.h>
#include <Adafruit_Sensor.h>

#include "BluefruitConfig.h"
#include <Adafruit_BLE.h>
#include <Adafruit_BluefruitLE_UART.h>

#include <SoftwareSerial.h>

#define MMA8451_INTR 9

#define PACKET_SIZE 40

// ------------------------ GLOBAL VARIABLE DECLARATIONS ------------------------

// MMA8451 variables
Adafruit_MMA8451 mma = Adafruit_MMA8451();
uint16_t xyz_buffer[3];

uint16_t timer = 0;
uint16_t timer_tmp = 0;

// Distress Flags
bool horizontal_long_flag = 0, orientation_change = 0;

// BLE variables
Adafruit_BluefruitLE_UART ble(Serial1, BLUEFRUIT_UART_MODE_PIN);
char update_packet[PACKET_SIZE + 1];
char *packet_ptr = update_packet;

//---------------------------------------------------------------------------------

 void setup() {
  
  Wire.begin(); // Join the bus

  // SERIAL COMMUNICATION USED FOR DEBUGGING
  // DO NOT USE SERIAL COMMUNICATION DURING I2C TRANSACTIONS!!!
  Serial.begin(115200);
  while(!Serial);
//  Serial.println("SERIAL COMMUNICATION INITIATED: 9600 BAUD");
  
//  Serial.println("FLORA JOINED BUS AS MASTER");

  if(!ble.begin(VERBOSE_MODE)) {
    Serial.println("Couldn't find Bluefruit");
  } else {Serial.println("Found Bluefruit");}
  ble.info();
/*
  while(!ble.isConnected()) {
    delay(500);
    Serial.println("Bluetooth Not Connected");
  }
  Serial.println("Bluetooth Connected");
*/
  // INITIALIZE THE MMA8451 ACCELEROMETER
  mma.begin();
  mma.setRange(MMA8451_RANGE_2_G);
  mma.setDataRate(MMA8451_DATARATE_1_56_HZ);
  // pinMode(MMA8451_INTR, INPUT); // This may or may not be used...
//  Serial.println("INITIALIZED MMA8451 ACCEL");
  delay(100);

  mma.read();
 }

char test_arr[5] = { 't','e', 's', 't', '\n' };

 void loop() {

  // Get a new sample from the MMA8451
  xyz_buffer[0] = mma.x;
  xyz_buffer[1] = mma.y;
  xyz_buffer[2] = mma.z;
  mma.read();

//--------------------- CHECK FOR CONDITIONS OF STRESS -------------------
  
//------------------------------------------------------------------------

//-------------------- DISPLAY DATA FOR DEBUGGING ------------------------
  Serial.print("X: "); Serial.print(mma.x);
  Serial.print(" -- Y: "); Serial.print(mma.y);
  Serial.print(" -- Z: "); Serial.print(mma.z);
  Serial.println("\n-----------------------------");
//------------------------------------------------------------------------

//--------------------- Bluetooth Test -----------------------------------
  ble.print("AT+BLEUARTTX=");
  ble.println(test_arr);
//------------------------------------------------------------------------
delay(1000);
//---------------------- DECIDE TO TRANSMIT DATA -------------------------
/*
 * if( hr_abnormal || spo2_abnormal || horizontal_long_flag || orientation_change || timer > 5 ) {
 *   timer = 0;
 *   // Assemble packet  
 *   *( (uint32_t*) packet_ptr ) = avg_hr;
 *   *( (uint32_t*) &packet_ptr[4] ) = avg_spo2;
 *   *( (uint32_t*) &packet_ptr[8] ) = xyz_buffer[0];
 *   *( (uint32_t*) &packet_ptr[12] ) = xyz_buffer[1];
 *   *( (uint32_t*) &packet_ptr[16] ) = xyz_buffer[2];
 *   *( (uint32_t*) &packet_ptr[20] ) = mma.x;
 *   *( (uint32_t*) &packet_ptr[24] ) = mma.y;
 *   *( (uint32_t*) &packet_ptr[28] ) = mma.z;
 *   *( (bool*) &packet_ptr[32] ) = hr_abnormal;
 *   *( (bool*) &packet_ptr[33] ) = spo2_abnormal;
 *   *( (bool*) &packet_ptr[34] ) = horizontal_long_flag;
 *   *( (bool*) &packet_ptr[35] ) = orientation_change;
 *   
 *   // Send to the BLE board
 *   ble.print("AT+BLEUARTTX=");
 *   ble.println(update_packet);
 *   
 *   // Protocol for a dropped packet???
 * }
 */
//------------------------------------------------------------------------

 }

