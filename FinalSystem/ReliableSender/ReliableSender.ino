#include <Wire.h>                   // Library to use I2C communucation.
#include <RTClibExtended.h>         // Library for the clock
#include <timeAndAlarm.h>           // Functions for handeling timer and alarms
#include <TinyGPS++.h>              // Handling GPS data
#include <EEPROM.h>                 // Administrating writing to/reading from EEPROM
#include <SDI12.h>                  // Library for SDI-12 communication
#include <avr/io.h>
#include <Adafruit_Sensor.h>        // Library that some of Adafruit's sensors uses.
#include <Adafruit_BMP280.h>        // Temperature and air pressure readings from the BMP280 chip.
#include <Adafruit_AM2320.h>        // Humidity sensor
#include "systemConstants.h"        // Constants for the system

// LoRa: 
#include <RHReliableDatagram.h>
#include <LoRa.h>
#include <RH_RF95.h>
#include <SPI.h>
RH_RF95 lora(LoRa_CS, LoRa_INT);                              // Instanciate the LoRA driver
RHReliableDatagram manager(lora, SENDER_ADDRESS); // Instanciate the reliable sender

// Dynamic memory for receiving messages:
uint8_t packageNum; 




RTC_DS3231 RTC;
timeAndAlarm TimeAlarm(RTC);
TinyGPSPlus gps;  //This is the GPS object that will pretty much do all the grunt work with the NMEA data


void setup() {
  Serial.begin(9600);
 
  setAwakePinConfig();
  initializePacketNum();
  activateClock();
  initializeAlarm(); // Clear previous alarms and turn off SQW
  DateTime timeNow = RTC.now();

  pinMode(INIT_SWITCH, INPUT);
  if (!digitalRead(INIT_SWITCH)){ // || (!(timeNow.month() % 6 ) && EEPROM.read(801) != timeNow.month())){ 
    // Button is being pressed
    // OR the month indicates we should reboot, and last reboot was not this month
    activateClock();
    //turnOnGPS();
    Serial.println("GPS Start");//Just show to the monitor that the sketch has started
    delay(100);
    if (updateClock(2)){
      // If the gps successfully got connection
      getPosition();
      EEPROM.update(801, timeNow.month());  // Indicating last month of clock-update
    }
    //turnOffGPS();
  }

  // Temp functions: 
  updateClock(1);
  ////

  
  TimeAlarm.setWakeUpPeriod(0, 1, 1);
  TimeAlarm.setAlarm1(1, 2*(SENDER_ADDRESS-2) +20);
  printAlarm();
  Serial.println(F("Sender initiated"));
  delay(100);
}


void loop() {
  goToSafeSleep();
  Serial.println(F("\nWoke up from sleep"));
  
  setAwakePinConfig();
  activateClock();
  
  ////////////////////// Data acquisition /////////////////////////
  /////////////////////////////////////////////////////////////////
  uint8_t data[18];                                              //
  fillLongIntToPos(packageNum, 1, 0, data);                      // Filling packet number to first byte
                                                                 //
  BMP.begin();                                                   //
  delay(40);                                                     //
  fillLongIntToPos(getTemperature(), 2, 1, data);                // Temp data byte 1-2
  Serial.println(F("Data collected"));                           //
                                                                 //
  activateHumiditySensor();                                      //
  fillLongIntToPos(getHumidity(), 2, 3, data);                   // Humidity data byte 3-4
  deactivateHumiditySensor();                                    //                                                               
  initializeWaterPressureSensor();                               //
                                                                 //
  fillPositionData(data);                                        // Position data byte 5-10 (was 15-20)
                                                                 //
  activateClock();                                               //
  fillLongIntToPos(TimeAlarm.getTimeStamp(), 4, 11, data);       // Filling time data to byte 11-14 
                                                                 //
  SDI12 mySDI12(depthDataPin);                                   // Define the SDI-12 bus
  PORTD |= B00100000;                                            //
  fillLongIntToPos(getDepth(), 3, 15, data);                     // Filling depth data to byte 15-17 (was 8-10) 
  PORTD &= B11011111;                                            //
                                                                 //
  /////////////////////////////////////////////////////////////////
  
  setAwakePinConfig();
  activateClock();
  initializeLoRa();
  if (!manager.sendtoWait(data, sizeof(data), RECEIVER_ADDRESS)){     
    Serial.println(F("Send unsuccessful")); 
    storeInEEPROM(data, sizeof(data));         
  }

  // receiveTimeoutMessage should make it possible to adjust the time that the sender wakes up
  // However, there is a bug...
  // When we receive a timeout message, something happens, which causes the system to bug up 
  // Not when receiving, but later, when we try to send from memory, when using "setAwakeConfig" or "setSleepConfig"
  // The problem has nothing to do with the function adjustWakeUpTime
  // Seems to have something to do with the pin settings, but PCMSK0 register seems unchanged, together with DDRx and PORTx
  // PCICR and SREG are also unchanged, still worth checking out to be sure maybe
  // It's weird, because the receiver is able to first receive and then send later on, 
  // but here it apparently messes thing up 
  /*
  int seconds = receiveTimeoutMessage(300); // Waiting 300 ms to check if we need to adjust the clock 
  if (seconds){
    TimeAlarm.adjustWakeUpTime(seconds); 
  }
  */
  
  sendFromEEPROM();
  updatePackageNum();
  
  TimeAlarm.setNextWakeupTime();
  TimeAlarm.setAlarm1();
  Serial.print(F("Going to sleep, "));
  printAlarm();
  delay(100);
}






























