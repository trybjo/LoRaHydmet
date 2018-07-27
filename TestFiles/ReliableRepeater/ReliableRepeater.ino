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

#define REPEATER_ADDRESS 6


RTC_DS3231 RTC;                                               // Instanciate the external clock
TinyGPSPlus gps;                                              // Instanciate the GPS
timeAndAlarm TimeAlarm(RTC);                                  // Instanciate the time and alarm handler

RH_RF95 lora(LoRa_CS, LoRa_INT);                              // Instanciate the LoRA driver
RH_Repeater_ReliableDatagram manager(lora, REPEATER_ADDRESS); // Instanciate the reliable sender


// Dynamic memory for receiving messages:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

void setup() {
  Serial.begin(9600);
  while (!Serial); // Wait for serial port to be available
  if (!manager.init())
  Serial.println(F("init failed"));
  delay(1000); 
  
  initializeLoRa();
  if (initializeMemory()){
    Serial.println(F("First time, initializing values to zero"));
    delay(50);
  }
  updateClock(1);
  Serial.println(TimeAlarm.getTimeStamp());
  initializeAlarm();
  // hh, mm, ss
  
  TimeAlarm.setWakeUpPeriod(0, 5, 0);

  // Set alarm 30 seconds before the next 10-minute:
  TimeAlarm.setAlarm1(5, -30);
  printAlarm();
  Serial.println(F("Setup repeater completed"));
  delay(100);
  goToSleep();
}

void loop() {

  TimeAlarm.stopAlarm();
  printAlarm();
  
  Serial.print(F("Woke up at time :"));
  Serial.println(TimeAlarm.getTime());
  
  byte firstReceiveFromSender = 0; // Byte is filled with ones for each sender 
  // that has send messages. Used to time the first message received
  while(TimeAlarm.timeDifference() > -60){ 
    // Receiving for one minute
    // Negative time difference means the alarm has gone off already
    // As the time since alarm increases, the difference becomes more negative
    
    if (manager.available()){
      uint8_t bufLen = sizeof(buf); // Needs to be here to convert to uint8_t
      uint8_t from;  // from becomes author of the message
      uint8_t to; // to becomes the intended receiver of the message
      bool duplicate;
      // receive(bool, message, bufLen, from, timeout);
      bool receiveSuccess = myReceive(duplicate, buf, &bufLen, &from, &to, 2000);

      // This following should make adjusting time possible
      /*
      if (receiveSuccess && !((firstReceiveFromSender >> from) & 1)){
        // If this is the first message we get from the given sender
        firstReceiveFromSender |= (1<<from) // Indicate that we now have got a message from the sender
        if (abs(TimeAlarm.timeDifference() + 30+ 2*from) > 15){
          // The time difference from now and when the sender should send message
          // Is larger than 15 seconds. 
          // We should send a message to that sender, and ask to change clock time
          uint8_t message[1];
          message[0] = TimeAlarm.timeDifference() + 2*from;  
          manager.sendtoWaitRepeater(&message[0], sizeof(message), from, REPEATER_ADDRESS);
        }        
      }
      */
      
      if (receiveSuccess && !duplicate && from != RECEIVER_ADDRESS){        
        // We only store messages from the senders             
        writeMessageToMemory(buf, bufLen, from); 
        printMemory(from);        
      }
      else if (receiveSuccess && !duplicate && from == RECEIVER_ADDRESS){
        forwardMessage(buf, bufLen, from, to);
        writeMessageToMemory(buf, bufLen, from);
      }
    }
  }
  
  Serial.print(F("Stopped listening at :"));
  
  Serial.println(TimeAlarm.getTime());
  
  sendFromMemory();
  
  DateTime timeNow = RTC.now();
  if (!(timeNow.day() % 7 ) && EEPROM.read(800) != timeNow.day()){
    // If the date number is dividable by 7
    // and if this date is last reboot date
    // Reboot
  }
  if (!(timeNow.month() % 6 ) && EEPROM.read(801) != timeNow.month()){
    // If the month is dividable by 6
    // and if this month is not last clock update month
    updateClock(2);
  }
  
  TimeAlarm.setNextWakeupTime();
  TimeAlarm.setAlarm1();

  printAlarm();  
  
  goToSafeSleep();
}



