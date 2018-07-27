#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <SPI.h>
#include <LoRa.h>
#include "systemConstants.h"


RH_RF95 lora(LoRa_CS, LoRa_INT);

RHReliableDatagram manager(lora, RECEIVER_ADDRESS); // Instanciate this object with address.
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
uint8_t packageMemory[4]; 
uint8_t packageMemoryPointer;
uint8_t packageNum;



void setup() {
  Serial.begin(9600);
  delay(100);
  
  while (!Serial) ; // Wait for serial port to be available
  initializeLoRa();
  for (int i = 0; i < 4; i++){
    packageMemory[i] = -1;
  }
  packageNum = 0;
  packageMemoryPointer = 0;
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
    // The variable 'duplicate' is not in use yet.
    Serial.print(F("From: "));
    Serial.print(from);
    Serial.print(", ");
    writeDataToSerial(buf);
    
    /*
    // After successful receive(s), send a message: 
    if (receiveSuccess){
      uint8_t data[] = "And hello back to you";
      uint8_t numberedResponse[sizeof(packageNum) + sizeof(data)];
      addPackageNum(&numberedResponse[0], &data[0], sizeof(data));
      
      if (!manager.sendtoWait(numberedResponse, sizeof(numberedResponse), from)){
        // Serial.println("Sending failed");
      }
      else {
        // Serial.print(F("Sending successful: "));
        // Serial.print((int)numberedResponse[0]);
        // Serial.println((char*)&numberedResponse[1]);
        updatePackageNum();
      }
    }
    */
  }
}
