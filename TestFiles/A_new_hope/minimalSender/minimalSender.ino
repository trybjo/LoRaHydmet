#include <Wire.h>                   // Library to use I2C communucation.
#include <RTClibExtended.h>         // Library for the clock
#include <timeAndAlarm.h>           // Functions for handeling timer and alarms
#include <TinyGPS++.h>              // Handling GPS data
#include <EEPROM.h>                 // Administrating writing to/reading from EEPROM
#include <SDI12.h>                  // Library for SDI-12 communication

#include "systemConstants.h"        // Constants for the system
RTC_DS3231 RTC;
timeAndAlarm TimeAlarm(RTC);
TinyGPSPlus gps;  //This is the GPS object that will pretty much do all the grunt work with the NMEA data


int i = 0;

void setup() {
  Serial.begin(9600);

  initializeAlarm(); // Clear previous alarms and turn off SQW
  DateTime timeNow = RTC.now();
  
  initializeWaterPressureSensor();

  Serial.println(F("Going to update clock"));
  updateClock(1);
  //getPosition();

  TimeAlarm.setWakeUpPeriod(0, 1, 0);
  TimeAlarm.setAlarm1(1, 2*(SENDER_ADDRESS-2));
  printAlarm();

}


void loop() {
 Serial.print(F("Running the loop for the "));
 Serial.print(i);
 Serial.println(F(". time"));
 i++;

 getDepth();
 
 delay(500);
 goToSafeSleep();
 
 TimeAlarm.setNextWakeupTime();
 TimeAlarm.setAlarm1();
 printAlarm();
 
}

