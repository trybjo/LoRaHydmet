#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <SPI.h>
#include <LoRa.h>
#include "systemConstants.h"
#include <memoryReceiver.h>
#include <EEPROM.h>
#include <Wire.h>


RH_RF95 lora(LoRa_CS, LoRa_INT);
RHReliableDatagram manager(lora, RECEIVER_ADDRESS); // Instanciate this object with address.

uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];


void setup() {
  Serial.begin(9600);
  delay(100);
  
  while (!Serial) ; // Wait for serial port to be available
  initializeLoRa();
  if (initializeMemory()){
    Serial.println(F("First time, initializing values to zero"));
    delay(50);
  }
  /*
  for (int i = 0; i < 4; i++){
    packageMemory[i] = -1;
  }
  packageNum = 0;
  packageMemoryPointer = 0;
  */
  //Serial.println(F("Signal Strength, Packet number, Temp, Humidity, Pressure, Debth, Time"));
  Serial.println(F("Receiver on"));
}



void loop() {
  if (manager.available()){
    // Serial.println();
    uint8_t bufLen = sizeof(buf); // Needs to be here to convert to uint8_t
    uint8_t from;
    bool duplicate;
    
    // We receive, and wait forever to receive it
    bool receiveSuccess = receive(&duplicate, buf, &bufLen, &from, -1);
    if (!receiveSuccess){
      Serial.println("Receive not a success");
    }
    else if (duplicate && from != REPEATER_ADDRESS){
      Serial.println(F("We got a duplicate: "));
      writeDataToSerial(buf, (int)bufLen, from);
    }
    else if (receiveSuccess && !duplicate && from != REPEATER_ADDRESS){
      writeMessageToMemory(buf, (int)bufLen, from);
      writeDataToSerial(buf, (int)bufLen, from);
    }
    else if (receiveSuccess && from == REPEATER_ADDRESS){
      Serial.print(F("Received message from repeater, RRSI: "));
      Serial.println((int)lora.lastRssi());
    }
    // The variable 'duplicate' is not in use yet.
    
  }
}
