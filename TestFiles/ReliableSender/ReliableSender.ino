 
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
uint8_t packageMemory[4]; 
uint8_t packageMemoryPointer;



//------------------------------------------------------------------------------------------------------------------------

void setup() 
{
   
  Serial.begin(9600);
  delay(10);
  updateClock(1);
  initializeAlarm();
  initializeLoRa();
  initializeTempAndPressure();
  initializeHumidity();
  initializeWaterPressureSensor();
  initializePacketNum();

  TimeAlarm.setWakeUpPeriod(0, 5, 0);
  // Set alarm the next 10-minute:
  TimeAlarm.setAlarm1(5, 2*SENDER_ADDRESS);
  printAlarm();
  Serial.println("Sender on");
  delay(100);
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
  
  uint8_t data[15]; // Uint8_t [2] can hold values in range 0-65'536
  // get() functions return long int. Data requires 15 bytes.
  fillLongIntToPos((long int) packageNum, 1, 0, data); // Adding packet number and adds sensor data to data.
  fillLongIntToPos(getTemperature(), 2, 1, data); // Temp data (2 bytes)
  fillLongIntToPos(getHumidity(), 2, 3, data); // Humidity data (2 byte)
  fillLongIntToPos(getPressure(), 3, 5, data); // Pressure data (3 bytes)
  fillLongIntToPos(getDepth(), 3, 8, data); // Depth data  (3 bytes)
  fillLongIntToPos(getTime(), 4, 11, data); // Time data (4 bytes)
                                           // Position data (3 + 3 byte)
                                            
  
  Serial.print(F("\nMessage generated, with number: "));
  Serial.println((int)data[0]); // Printing the first part of the message
  
  
  
  // Send newly generated data
  if (!manager.sendtoWait(data, sizeof(data), RECEIVER_ADDRESS))
  {     
    Serial.println(F("Send unsuccessful")); 
    storeInEEPROM(data, sizeof(data));         
  }
  // This following should make adjusting time possible
  /*
  uint8_t bufLen = sizeof(buf); // Needs to be here to convert to uint8_t
  uint8_t from;  // from becomes author of the message
  uint8_t to; // to becomes the intended receiver of the message
  if (manager.recvfromAckTimeout(buf, bufLen, 1000, from, to)){
    if (from == REPEATER_ADDRESS && bufLen == 1){
      TimeAlarm.adjustWakeUpTime(buf[0]);
    }
  }
  */
  
  Serial.print(F("RSSI [dBm] of last received message  : "));
  Serial.println((int)lora.lastRssi());
  
  sendFromEEPROM();
  
  //printAllEEPROM(); 
  updatePackageNum();

  TimeAlarm.setNextWakeupTime();
  TimeAlarm.setAlarm1();
  
  printAlarm();

  goToSafeSleep();
  
}

