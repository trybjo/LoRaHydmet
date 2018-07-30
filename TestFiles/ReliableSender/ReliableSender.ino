 
 // Libraries:
#include <LoRa.h>                   // For changing transmission settings
#include <RH_RF95.h>                // Base radio communication functions
#include <RHReliableDatagram.h>     // Funcitons for communication with acknowledgement   
#include <EEPROM.h>                 // Administrating writing to/reading from EEPROM
#include <RTClibExtended.h>         // Library for the clock
#include <TinyGPS++.h>              // Handling GPS data
#include <Adafruit_Sensor.h>        // Library that some of Adafruit's sensors uses.
#include <Adafruit_BMP280.h>        // Temperature and air pressure readings from the BMP280 chip.
#include <Adafruit_AM2320.h>        // Humidity sensor
#include <SPI.h>                    // SPI library easy use of SPI. MISO, MOIS and SCK hare handled automatically by the SPI-library. Chip select (CS)/Slave select (SS) must be set manually. CS/SS pin can be any digital pin. The slave communicates with master when its CS pin is low (usually...).
#include <Wire.h>                   // Library to use I2C communucation.
#include <SDI12.h>                  // Library for SDI-12 communication
#include <avr/wdt.h>                // Library for watchdog timer.
#include <timeAndAlarm.h>           // Functions for handeling timer and alarms

#include "systemConstants.h"        // Constants for the system

TinyGPSPlus gps;//This is the GPS object that will pretty much do all the grunt work with the NMEA data
RTC_DS3231 RTC;
timeAndAlarm TimeAlarm(RTC);
Adafruit_AM2320 am2320 = Adafruit_AM2320(); // Instance for the humidity sensor.


// Variables for packet handling
uint8_t packageNum; 
// Don't put this on the stack:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

/*
uint8_t packageMemory[4]; 
uint8_t packageMemoryPointer;
*/
bool buttonPress = false;

//------------------------------------------------------------------------------------------------------------------------

void setup() 
{
  while(buttonPress){
    for (int i = 0; i < 1000; i++){
      EEPROM.update(i, 0);
    }
  }
  
  turnOffWatchdogTimer();
  
  Serial.begin(9600);
  delay(10);
  EEPROM.update(98, 0); // Temporary in order to get time every boot.
  
  if (!EEPROM.read(98)){
    // We have not started the sender before, we need to set the time
    updateClock(1);
  }
  
  
  initializeAlarm();
  initializeLoRa();
  initializeTempAndPressure();
  initializeHumidity();
  initializeWaterPressureSensor();
  initializePacketNum();

  DateTime timeNow = RTC.now();
  EEPROM.update(800, timeNow.day());      // Indicating last date of reboot

  TimeAlarm.setWakeUpPeriod(0, 3, 1);
  // Set alarm the next 10-minute:
  TimeAlarm.setAlarm1(3, 15 + 2*SENDER_ADDRESS);
  printAlarm();

  Serial.print(F("Time: "));
  Serial.println(TimeAlarm.getTimeStamp());
  Serial.println(F("Sender on"));
  delay(100);
  EEPROM.update(98, 1);
  goToSleep();
}

//------------------------------------------------------------------------------------------------------------------------

void loop() 
{    
  
  printAlarm();
  Serial.print(F("Woke up at time :"));
  Serial.println(TimeAlarm.getTime());
  delay(2000);
  TimeAlarm.stopAlarm();
  
  uint8_t data[20]; // Uint8_t [2] can hold values in range 0-65'536
  // get() functions return long int. Data requires 15 bytes.
  fillLongIntToPos((long int) packageNum, 1, 0, data); // Adding packet number and adds sensor data to data.
  fillLongIntToPos(getTemperature(), 2, 1, data); // Temp data (2 bytes)
  fillLongIntToPos(getHumidity(), 2, 3, data);  // Humidity data (2 byte)
  fillLongIntToPos(getPressure(), 3, 5, data);  // Pressure data (3 bytes)
  fillLongIntToPos(getDepth(), 3, 8, data);     // Depth data  (3 bytes)
  fillLongIntToPos(getTime(), 4, 11, data);     // Time data (4 bytes)
  long int Lat;
  long int Lng;
  //getPosition(Lat, Lng);
  //fillLongIntToPos(Lat, 3, 15, data);
  //fillLongIntToPos(Lng, 3, 18, data);           // Position data (3 + 3 byte)
                                           
                                            
  
  Serial.print(F("\nMessage generated, with number: "));
  Serial.println((int)data[0]); // Printing the first part of the message
  
  
  
  // Send newly generated data
  if (!manager.sendtoWait(data, sizeof(data), RECEIVER_ADDRESS))
  {     
    Serial.println(F("Send unsuccessful")); 
    storeInEEPROM(data, sizeof(data));         
  }
  Serial.println(F("Done with sending"));
  
  // This following should make adjusting time possible
  
  uint8_t bufLen = sizeof(buf); // Needs to be here to convert to uint8_t
  uint8_t from;  // from becomes author of the message
  
  if (manager.recvfromAckTimeout(buf, &bufLen, 1000, &from)){
    if (from == REPEATER_ADDRESS && bufLen == 2){
      Serial.print(F("We got time request: "));
      int seconds = buf[1];
      if (!buf[0]){
        // Position 0 is 0 if the value of seconds is negative 
        seconds = seconds * -1; 
      }
      Serial.println(seconds);
      TimeAlarm.adjustWakeUpTime(seconds);
    }
  }
  
  
  Serial.print(F("RSSI [dBm] of last received message  : "));
  Serial.println((int)lora.lastRssi());
  
  sendFromEEPROM();
  
  //printAllEEPROM(); 
  updatePackageNum();

  TimeAlarm.setNextWakeupTime();
  TimeAlarm.setAlarm1();
  
  printAlarm();

  
  DateTime timeNow = RTC.now();
  if (!(timeNow.day() % 7 ) && EEPROM.read(800) != timeNow.day()){
    // If the date number is dividable by 7
    // and if this date is last reboot date
    // Reboot
    void rebootMCU();
  }
  if (!(timeNow.month() % 6 ) && EEPROM.read(801) != timeNow.month()){
    // If the month is dividable by 6
    // and if this month is not last clock update month
    updateClock(2);
    EEPROM.update(801, timeNow.month());
  }

  goToSafeSleep();
  
}

