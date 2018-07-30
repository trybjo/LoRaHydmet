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


RTC_DS3231 RTC;                                               // Instanciate the external clock
TinyGPSPlus gps;                                              // Instanciate the GPS
timeAndAlarm TimeAlarm(RTC);                                  // Instanciate the time and alarm handler

RH_RF95 lora(LoRa_CS, LoRa_INT);                              // Instanciate the LoRA driver
RH_Repeater_ReliableDatagram manager(lora, REPEATER_ADDRESS); // Instanciate the reliable sender


// Dynamic memory for receiving messages:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

void setup() {
  turnOffWatchdogTimer();
  
  Serial.begin(9600);
  while (!Serial); // Wait for serial port to be available
  

  DateTime timeNow = RTC.now();
  EEPROM.update(800, timeNow.day());      // Indicating last date of reboot
  
  initializeLoRa();
  if (initializeMemory()){
    // First time starting the repeater
    // updateClock(2);
    Serial.println(F("First time, initializing values to zero"));
    delay(50);
  }
  updateClock(1);
  Serial.println(TimeAlarm.getTimeStamp());
  initializeAlarm();

  TimeAlarm.setWakeUpPeriod(0, 3, 0);  
  TimeAlarm.setAlarm1(3, -30); // Set alarm 30 seconds before the next 5-minute:
  
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
  while(TimeAlarm.timeDifference() > -90){ 
    // Receiving for one minute
    // Negative time difference means the alarm has gone off already
    // As the time since alarm increases, the difference becomes more negative
    
    if (manager.available()){
      uint8_t bufLen = sizeof(buf); // Needs to be here to convert to uint8_t
      uint8_t from;  // from becomes author of the message
      uint8_t to; // to becomes the intended receiver of the message
      bool duplicate;
      // receive(bool, message, bufLen, from, timeout);
      bool receiveSuccess = myReceive(duplicate, buf, &bufLen, &from, &to, 300);

      // This following should make adjusting time possible
      
      Serial.print(F("Received a message at time: "));
      Serial.println(TimeAlarm.getTime());
      if (receiveSuccess && !((firstReceiveFromSender >> from) & 1)){
        // If this is the first message we get from the given sender
        firstReceiveFromSender |= (1<<from); // Indicate that we now have got a message from the sender
        int timeDiff = TimeAlarm.timeDifference() + 30 + 2*from;
        if (abs(timeDiff) > 15){
          // The time difference from now and when the sender should send message
          // Is larger than 15 seconds. 
          // We should send a message to that sender, and ask to change clock time
          Serial.print(F("We got a message too late: "));
          Serial.println(timeDiff);
          // Alternative: 
          uint8_t message[2];
          if (timeDiff < 0){
            message[0] = 0;
          }
          else{
            message[0] = 1;
          }
          message[1] = abs(timeDiff);
          // Alternative end
          manager.sendtoWaitRepeater(message, sizeof(message), from, (uint8_t)REPEATER_ADDRESS);
        }        
      }
      else{
        Serial.println(F("This is not the first from this sender"));
      }
      
      
      
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
    EEPROM.update(801, timeNow.month());
  }
  
  TimeAlarm.setNextWakeupTime();
  TimeAlarm.setAlarm1();

  printAlarm();  
  
  goToSafeSleep();
}



