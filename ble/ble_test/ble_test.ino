
#include "BluefruitConfig.h"
#include <Adafruit_BLE.h>
#include<Adafruit_BluefruitLE_UART.h>

#include <SoftwareSerial.h>

#define PACKET_SIZE 40

Adafruit_BluefruitLE_UART ble(Serial1, BLUEFRUIT_UART_MODE_PIN);
char update_packet[PACKET_SIZE + 1];
char *packet_ptr = update_packet;

void setup() {

  while(!Serial);
  Serial.begin(115200);

  if(!ble.begin()) {
    Serial.println("Could not find Bluefruit");
  } else {
    Serial.println("Found Bluefruit");
  }
  ble.info();
  
}

char test_arr[5] = {'t', 'e', 's', 't', '\n'};

void loop() {

ble.print("AT+BLEUARTTX=");
ble.println(test_arr);
delay(1000);
  
}

