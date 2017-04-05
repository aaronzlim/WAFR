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
 * Version: 1.0.0 First Release
 * 
 */

#include <Arduino.h>
#include <TinyWireM.h>
#include <USI_TWI_Master.h>
#include <Wire.h>

#include <Adafruit_MMA8451.h>
#include <Adafruit_Sensor.h>

#include "max30102_wire_driver.h"
#include "max30102_processing.h"

#include "BluefruitConfig.h"
#include <Adafruit_BLE.h>
#include <Adafruit_BluefruitLE_UART.h>

#include <SoftwareSerial.h>

#define MAX30102_INTR 10 // The MAX30102 interrupt line will go to pin 10 on the Flora
#define MMA8451_INTR 9

#define PACKET_SIZE 40

// ------------------------ GLOBAL VARIABLE DECLARATIONS ------------------------

// BLE variables
//Adafruit_BluefruitLE_UART ble(Serial1, BLUEFRUIT_UART_MODE_PIN);
char update_packet[PACKET_SIZE + 1];
char *packet_ptr = update_packet;

// MMA8451 variables
Adafruit_MMA8451 mma = Adafruit_MMA8451();
uint16_t xyz_buffer[3];

// MAX30102 DATA COLLECTION
uint32_t red_buffer[BUFFER_SIZE]; // red LED sensor data
uint32_t ir_buffer[BUFFER_SIZE]; // infrared LED sensor data

// HR CALCULATION
uint32_t num_peaks_arr[INITIAL_SAMPLE_SIZE];
uint32_t heart_rate_arr[INITIAL_SAMPLE_SIZE];
uint32_t total_num_peaks = 0;
int32_t heart_rate, avg_hr; // heart rate value
bool hr_valid; // indicator if heart rate calculation is valid

// SPO2 CALCULATION
uint32_t spo2_ratio_arr[INITIAL_SAMPLE_SIZE];
uint32_t spo2_arr[INITIAL_SAMPLE_SIZE];
float avg_ratio = 0;
uint32_t avg_spo2 = 0;
int32_t spo2; // SPO2 value
bool spo2_valid; // indicator to show if the SPO2 calculation is valid

uint32_t i, j; // incrementor

uint16_t timer = 0;
uint16_t timer_tmp = 0;

// Distress Flags
bool hr_abnormal = 0, spo2_abnormal = 0;
bool horizontal_long_flag = 0, orientation_change = 0;

//---------------------------------------------------------------------------------

 void setup() {
  
  Wire.begin(); // Join the bus

  // SERIAL COMMUNICATION USED FOR DEBUGGING
  // DO NOT USE SERIAL COMMUNICATION DURING I2C TRANSACTIONS!!!
  Serial.begin(9600);
  while(!Serial);
//  Serial.println("SERIAL COMMUNICATION INITIATED: 9600 BAUD");
  
//  Serial.println("FLORA JOINED BUS AS MASTER");

/*
  if(!ble.begin(VERBOSE_MODE)) {
    Serial.println(F("Couldn't find Bluefruit"));
  }
*/
  // INITIALIZE THE MMA8451 ACCELEROMETER
  mma.begin();
  mma.setRange(MMA8451_RANGE_2_G);
  mma.setDataRate(MMA8451_DATARATE_1_56_HZ);
  // pinMode(MMA8451_INTR, INPUT); // This may or may not be used...
//  Serial.println("INITIALIZED MMA8451 ACCEL");
  delay(100);

  // INITIALIZE the MAX30102 PULSE OXIMETER
  max30102_reset();
  pinMode(MAX30102_INTR, INPUT);
  // Read/Clear the interrupt status registers
  max30102_clear_interrupt_status_regs();
  // Setup all max30102 registers
  max30102_init();
  delay(100);
//  Serial.println("INITIALIZED MAX30102 PULSE OXIMETER");
//  Serial.println("STARTING FIRST BUFFER LOAD");
  delay(100);
//--------------------- MAX30102 FIRST BUFFER LOAD ------------------------
  // Put 5 num_peak samples in num_peaks_arr
  for(i = 0; i < INITIAL_SAMPLE_SIZE; i++) { // Collect 20s worth of peaks (heartbeats)
    // Collect 100 samples
    for(j=0; j < BUFFER_SIZE; j++) {
      while(digitalRead(MAX30102_INTR)==1); // Wait until the interrupt pin asserts
      // Read from max30102 FIFO
      if(!max30102_read_fifo(red_buffer, ir_buffer, j)) {
        j--; // If there is nothing to read decrement the incrementor and try again.
      }
    }
    num_peaks_arr[i] = get_num_peaks(ir_buffer);
    spo2_ratio_arr[i] = get_spo2_ratio(red_buffer, ir_buffer);
  }
//-------------------------------------------------------------------------

  num_peaks_arr[0] = (num_peaks_arr[1] + 
                      num_peaks_arr[2] + 
                      num_peaks_arr[3] + 
                      num_peaks_arr[4]) / 4; // First read is erroneous, so replace it                 

//---------------------- CALCULATE HR AND SPO2 -------------------------
  total_num_peaks = 0;
  for(i = 0; i < INITIAL_SAMPLE_SIZE; i++) {
    total_num_peaks += num_peaks_arr[i];
  }

  // CALCULATE HR
  heart_rate = PEAKS_TO_HR(total_num_peaks);
  if(heart_rate < 225 && heart_rate > 30) {
      avg_hr = 0;
      for(i = 0; i < INITIAL_SAMPLE_SIZE - 1; i++) { // Shift heart rate array left by one
        heart_rate_arr[i] = heart_rate_arr[i+1];
        avg_hr += heart_rate_arr[i];
      }
      heart_rate_arr[INITIAL_SAMPLE_SIZE - 1] = heart_rate; // Add new heart rate to heart rate array
      avg_hr = (avg_hr + heart_rate) / INITIAL_SAMPLE_SIZE; // Calculate average HR from heart rate array
      hr_valid = 1;
  }
  else { hr_valid = 0; }

  // CALCULATE SPO2
  for(i = 0; i < INITIAL_SAMPLE_SIZE; i++) { avg_ratio += spo2_ratio_arr[i]; }
  avg_ratio = (avg_ratio / (100 * INITIAL_SAMPLE_SIZE));
  spo2 = (uint32_t) ((-55.426 * avg_ratio * avg_ratio) + (50.129 * avg_ratio) + 89.612);
  if(spo2 <= 100 && spo2 >= 80) {
      avg_spo2 = 0;
      for(i = 0; i < INITIAL_SAMPLE_SIZE-1; i++){
        spo2_arr[i] = spo2_arr[i+1];
        avg_spo2 += spo2_arr[i];
      }
      spo2_arr[INITIAL_SAMPLE_SIZE-1] = spo2;
      avg_spo2 = (avg_spo2 + spo2) / INITIAL_SAMPLE_SIZE;
      spo2_valid = 1;
  }
  else {spo2_valid = 0;}  
// ----------------------------------------------------------------------

  // Initialize HR and SPO2 buffers
  for(i = 0; i < INITIAL_SAMPLE_SIZE; i++) {
    heart_rate_arr[i] = heart_rate; 
    spo2_arr[i] = spo2;
  }
  avg_hr = heart_rate;
  avg_hr = spo2;

  mma.read();
  
  delay(100);
//  Serial.println("MAX30102 FIRST DATA LOAD COMPLETE\n");
  delay(100);
 }

 void loop() {

  // Get a new sample from the MMA8451
  xyz_buffer[0] = mma.x;
  xyz_buffer[1] = mma.y;
  xyz_buffer[2] = mma.z;
  mma.read();

//--------------------- CHECK FOR CONDITIONS OF STRESS -------------------
  // These values are for testing, actual will be <40 and >165
  
  if( avg_hr < 70 || avg_hr > 80 ) { hr_abnormal = 1;} 
  else {hr_abnormal = 0;}
  if( avg_spo2 < 95 ) {spo2_abnormal = 1;}
  else {spo2_abnormal = 0; }
  

  /*
   * if( mma.z >= 7.5 m/s^2 || mma.z <= -7.5 m/s^2 ){
   * 
   *   if( abs(timer-timer_tmp) > 3 ) {
   *     // Wait 15-20 seconds and see if still horizontal
   *     // if so, then:
   *     horizontal_long_flag = 1;
   *     timer_tmp = timer;
   *   } else {horizontal_long_flag = 0;}
   *   
   * } else {horizontal_long_flag = 0;}
   * 
   * //calculate difference in xyz from previous loop
   * // mma.x - xyz_buffer[0], mma.y - xyz_buffer[1], mma.z - xyz_buffer[2]
   * // determine what orientation changes need to go in if/else statement(s)
   * 
   * if( ) {
   *   
   * } else {
   * 
   */
  
//------------------------------------------------------------------------

//-------------------- DISPLAY DATA FOR DEBUGGING ------------------------

  Serial.print("HR: "); Serial.print(avg_hr);
  Serial.print(" -- SPO2: "); Serial.print(avg_spo2);
  Serial.print(" -- X: "); Serial.print(mma.x);
  Serial.print(" -- Y: "); Serial.print(mma.y);
  Serial.print(" -- Z: "); Serial.print(mma.z);
  Serial.print(" -- HR_ABNORMAL: "); Serial.print(hr_abnormal);
  Serial.print(" -- SPO2_ABNORMAL: "); Serial.print(spo2_abnormal);
//  Serial.print(" -- HZL_LONG_FLAG: "); Serial.print(horizontal_long_flag);
//  Serial.print(" -- OR_CHANGE: "); Serial.print(orientation_change);
  Serial.println("\n--------------------------------------------------------");
//------------------------------------------------------------------------

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

//----------------- GET A NEW SAMPLE FROM MAX30102 --------------------

  // Collect 100 new samples (4 seconds)
  for(i = 0; i < BUFFER_SIZE; i++) {
    while(digitalRead(MAX30102_INTR) == 1); // Wait until the interrupt pin asserts
    if(!max30102_read_fifo(red_buffer, ir_buffer, i)) {
      i--; // If there is nothing to read decrement and try again
    }
  }

  for(i = 0; i < INITIAL_SAMPLE_SIZE - 1; i++) {
    num_peaks_arr[i] = num_peaks_arr[i+1];
    spo2_ratio_arr[i] = spo2_ratio_arr[i+1];
  }
  num_peaks_arr[INITIAL_SAMPLE_SIZE - 1] = get_num_peaks(ir_buffer);
  spo2_ratio_arr[INITIAL_SAMPLE_SIZE - 1] = get_spo2_ratio(red_buffer, ir_buffer);
  
//---------------------------------------------------------------------

  
  
//------------------- CALCULATE HR AND SPO2 ---------------------------
  total_num_peaks = 0;
  for(i = 0; i < INITIAL_SAMPLE_SIZE; i++) {
    total_num_peaks += num_peaks_arr[i];
  }

  // CALCULATE HR
  heart_rate = PEAKS_TO_HR(total_num_peaks);
  if(heart_rate < 225 && heart_rate > 30) {// && abs(heart_rate - avg_hr) < 30) {
      avg_hr = 0;
      for(i = 0; i < INITIAL_SAMPLE_SIZE - 1; i++) { // Shift heart rate array left by one
        heart_rate_arr[i] = heart_rate_arr[i+1];
        avg_hr += heart_rate_arr[i];
      }
      heart_rate_arr[INITIAL_SAMPLE_SIZE - 1] = heart_rate; // Add new heart rate to heart rate array
      avg_hr = (avg_hr + heart_rate) / INITIAL_SAMPLE_SIZE; // Calculate average HR from heart rate array
      hr_valid = 1;
  }
  else { hr_valid = 0; }

  // CALCULATE SPO2
  for(i = 0; i < INITIAL_SAMPLE_SIZE; i++) { avg_ratio += spo2_ratio_arr[i]; }
  avg_ratio = (avg_ratio / (100 * INITIAL_SAMPLE_SIZE));
  spo2 = (uint32_t) ((-55.426 * avg_ratio * avg_ratio) + (50.129 * avg_ratio) + 89.612);
  if(spo2 <= 100 && spo2 >= 80) {
      avg_spo2 = 0;
      for(i = 0; i < INITIAL_SAMPLE_SIZE-1; i++){
        spo2_arr[i] = spo2_arr[i+1];
        avg_spo2 += spo2_arr[i];
      }
      spo2_arr[INITIAL_SAMPLE_SIZE-1] = spo2;
      avg_spo2 = (avg_spo2 + spo2) / INITIAL_SAMPLE_SIZE;
      spo2_valid = 1;
  }
  else {spo2_valid = 0;}
//---------------------------------------------------------------------

 }

