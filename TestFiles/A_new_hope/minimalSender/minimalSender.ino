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
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
uint8_t packageNum; 
Adafruit_AM2320 am2320 = Adafruit_AM2320(); // Instance for the humidity sensor.



RTC_DS3231 RTC;
timeAndAlarm TimeAlarm(RTC);
TinyGPSPlus gps;  //This is the GPS object that will pretty much do all the grunt work with the NMEA data


void setup() {
  Serial.begin(9600);
 
  setAwakePinConfig();
  delay(40);
  initializePacketNum();
  //Serial.println(F("Turning GPS on"));
  activateClock();
  
  
  turnOnGPS();
  Serial.println(F("Activating gps: "));
  delay(100);
  delay(5000);
  turnOffGPS();
  
  

  
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
  setAwakePinConfig();
  activateClock();
  
  ////////////////////// Data acquisition /////////////////////////
  /////////////////////////////////////////////////////////////////
  uint8_t data[21];                                              //
  fillLongIntToPos(packageNum, 1, 0, data);                      // Filling packet number to first byte
                                                                 //
  initializeWaterPressureSensor();                               //
  SDI12 mySDI12(depthDataPin);                                   // Define the SDI-12 bus
  PORTD |= B00100000;                                            //
  fillLongIntToPos(getDepth(), 3, 8, data);                      // Filling depth data to byte 8-10
  PORTD &= B11011111;       

  BMP.begin();
  delay(40);
  fillLongIntToPos(getTemperature(), 2, 1, data); // Temp data (2 bytes)
  Serial.println(F("Data collected"));

  activateHumiditySensor();
  fillLongIntToPos(getHumidity(), 2, 3, data);  // Humidity data (2 byte)
  deactivateHumiditySensor();

  activateClock();
  fillLongIntToPos(TimeAlarm.getTimeStamp(), 4, 11, data);       // Filling time data to byte 11-14
  fillPositionData(data);                                        // Position data (3 + 3 byte)
  /////////////////////////////////////////////////////////////////
  
  
  initializeLoRa();
  if (!manager.sendtoWait(data, sizeof(data), RECEIVER_ADDRESS)){     
    Serial.println(F("Send unsuccessful")); 
    storeInEEPROM(data, sizeof(data));         
  }

  ///// Waiting 1 sec to check if we need to adjust the clock ////
  ////////////////////////////////////////////////////////////////
  uint8_t bufLen = sizeof(buf);                                 // Needs to be here to convert to uint8_t
  uint8_t from;                                                 // from becomes author of the message
  if (manager.recvfromAckTimeout(buf, &bufLen, 1000, &from)){   //
    if (from == REPEATER_ADDRESS && bufLen == 2){               // Special message from repeater 
      Serial.print(F("We got time request: "));                 //
      int seconds = buf[1];                                     // Number of seconds to change the clock
      if (!buf[0]){                                             //
        // Position 0 is 0 if the value of seconds is negative  //
        seconds = seconds * -1;                                 //
      }                                                         //
      Serial.println(seconds);                                  //
      TimeAlarm.adjustWakeUpTime(seconds);                      // Adjusting the clock forwards of backwards, depending on sign
    }                                                           //
  }                                                             //
  ////////////////////////////////////////////////////////////////
  
  sendFromEEPROM();
  updatePackageNum();
  
  Serial.println(F("Going to sleep"));
  delay(500);
  goToSafeSleep();
  Serial.println(F("\nWoke up from sleep"));
  
  TimeAlarm.setNextWakeupTime();
  TimeAlarm.setAlarm1();
  printAlarm();

}






























