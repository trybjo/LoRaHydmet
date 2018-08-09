#include <timeAndAlarm.h>
#include <memoryRepeater.h>
#include "systemConstants.h"

#include <RH_Repeater_ReliableDatagram.h>
#include <LoRa.h>
#include <RH_RF95.h>
#include <SPI.h>
#include <EEPROM.h>
#include "TinyGPS++.h"
#include "RTClibExtended.h"
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <avr/wdt.h>                // Library for watchdog timer.
#include <avr/io.h>


RTC_DS3231 RTC;                                               // Instanciate the external clock
TinyGPSPlus gps;                                              // Instanciate the GPS
timeAndAlarm TimeAlarm(RTC);                                  // Instanciate the time and alarm handler

RH_RF95 lora(LoRa_CS, LoRa_INT);                              // Instanciate the LoRA driver
RH_Repeater_ReliableDatagram manager(lora, REPEATER_ADDRESS); // Instanciate the reliable sender


// Dynamic memory for receiving messages:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

void setup() {
  turnOffWatchdogTimer();
  setAwakePinConfig();
  Serial.begin(9600);
  while (!Serial); // Wait for serial port to be available

  activateClock();
  DateTime timeNow = RTC.now();
  updateClock(1);
  whipeMemory();
  Serial.println(TimeAlarm.getTimeStamp());
  Serial.println(F("Setup repeater completed"));
  delay(100);
}

void loop() {
  setAwakePinConfig();
  activateClock();

  TimeAlarm.stopAlarm();
  initializeLoRa();

  /////////////////////////// Receiving for 2 seconds ////////////////////////////////
  bool receivedMessage = false;                                                     //
  for ( int i = 0; i < 100; i++){ // Receiving for 100 * 20 ms = 2 seconds          //
    if (manager.available()){                                                       //
      uint8_t bufLen = sizeof(buf); // Needs to be here to convert to uint8_t       //
      uint8_t from;                 // from becomes author of the message           //
      uint8_t to;                   // to becomes the intended receiver             //
      bool duplicate;                                                               //
                                                                                    //
      bool receiveSuccess = myReceive(duplicate, buf, &bufLen, &from, &to, 200);    //
                                                                                    //
      if (receiveSuccess && from != RECEIVER_ADDRESS){                //
        Serial.print(F("Received message from sender: "));                          //
        Serial.print(buf[0]);                                                       //
        Serial.print(F(", RRSI: "));                                                //
        Serial.println((int)lora.lastRssi());                                       //
        writeMessageToMemory(buf, bufLen, from);                                    //
        receivedMessage = true;                                                     //
        break;                                                                      //
      }                                                                             //
    }                                                                               //
    delay(20);                                                                      //
  }                                                                                 //
  ////////////////////////////////////////////////////////////////////////////////////
  if (!receivedMessage){
    Serial.println("Got no package from sender");
  }
  //////////////// Sending message if nothing received for the 2 seconds /////////////////////////////
  //if (!receivedMessage){                                                                          //
    uint8_t message[2];                                                                             //
    if (manager.sendtoWaitRepeater(message, sizeof(message), RECEIVER_ADDRESS, REPEATER_ADDRESS)){  //
      Serial.print(F("Message sent to the receiver with RRSI: "));                                  //
      Serial.println((int)lora.lastRssi());                                                         //
    }                                                                                               //
    else{                                                                                           //
      Serial.println(F("No connection with receiver"));                                             //
    }                                                                                               //
  //}                                                                                               //
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  sendFromMemory();
  Serial.println();
}



