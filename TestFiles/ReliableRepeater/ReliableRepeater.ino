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

  
  //DDRB &= ~(1 << (INIT_BUTTON - 8)); // Set pin 0 (digital 8) to input (0)
  //if (DDRB >> INIT_BUTTON - 8){ // Read digital pin 8
  pinMode(INIT_BUTTON, INPUT);
  if (digitalRead(INIT_BUTTON)){
    Serial.println("Here");
    // Button is being pressed
    // First time starting the repeater
    whipeMemory();
    activateClock();
    updateClock(2);
  }
  // Temporarily
  activateClock();
  updateClock(1);
  //////////////
  
  initializeLoRa();
  activateClock();
    
  DateTime timeNow = RTC.now();
  EEPROM.update(800, timeNow.day());      // Indicating last date of reboot
  
  initializeAlarm();

  TimeAlarm.setWakeUpPeriod(0, 3, 0);  
  TimeAlarm.setAlarm1(3, -30); // Set alarm 30 seconds before the next 3-minute:

  
  Serial.println(TimeAlarm.getTimeStamp());
  printAlarm();
  Serial.println(F("Setup repeater completed"));
  delay(100);
  
  goToSafeSleep();
}

void loop() {
  setAwakePinConfig();
  activateClock();

  TimeAlarm.stopAlarm();
  initializeLoRa();
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
      bool receiveSuccess = myReceive(duplicate, buf, &bufLen, &from, &to, 300);

      // This following should make adjusting time possible
      if (receiveSuccess){
        sendTimingError(firstReceiveFromSender, from);
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
  
  Serial.print(F("Stopped listening at: "));
  Serial.println(TimeAlarm.getTime());
  
  sendFromMemory();
  
  DateTime timeNow = RTC.now();
  if (!(timeNow.day() % 7 ) && EEPROM.read(800) != timeNow.day()){
    // If the date number is dividable by 7
    // and if this date is not last reboot date
    rebootMCU();
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



