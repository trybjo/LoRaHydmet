#include <Wire.h>                   // Library to use I2C communucation.
#include <RTClibExtended.h>         // Library for the clock
#include <timeAndAlarm.h>           // Functions for handeling timer and alarms
#include <TinyGPS++.h>              // Handling GPS data
#include <EEPROM.h>                 // Administrating writing to/reading from EEPROM
#include <SDI12.h>                  // Library for SDI-12 communication
#include <avr/io.h>

#include "systemConstants.h"        // Constants for the system
RTC_DS3231 RTC;
timeAndAlarm TimeAlarm(RTC);
TinyGPSPlus gps;  //This is the GPS object that will pretty much do all the grunt work with the NMEA data


int i = 0;

void setup() {
  Serial.begin(9600);

  // Set pinMode:
  DDRB &= B11000000; // Not touching crystal
  DDRC &= B10000100; // // Not touching unknown pin
  DDRC |= B00000100; // GPS MOSFET pin as OUTPUT
  DDRD &= B00110000; // 
  DDRD |= B00110000; // Clock power pin and Depth Multiplexer&MOSFET pin as output

  PORTB |= B00111111; // Not touching crystal
  PORTC |= B01111011; // Not touching unknown pin
  PORTC &= B11111011; // Setting GPS MOSFET pin LOW
  PORTD |= B11011111; // 
  PORTD &= B11011111; // Set Depth Multiplexer&MOSFET pin LOW
  
  delay(50);
  initializeAlarm(); // Clear previous alarms and turn off SQW
  DateTime timeNow = RTC.now();

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

 initializeWaterPressureSensor();
 SDI12 mySDI12(depthDataPin); // Define the SDI-12 bus
 PORTD |= B00100000;
 Serial.println(getDepth());
 PORTD &= B11011111;

 Serial.println(F("Going to sleep"));
 delay(500);
 goToSafeSleep();
 Serial.println(F("\nWoke up from sleep"));
 
 TimeAlarm.setNextWakeupTime();
 TimeAlarm.setAlarm1();
 printAlarm();
 
}

