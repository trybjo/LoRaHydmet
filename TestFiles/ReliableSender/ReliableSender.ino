 
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
#include <avr/io.h>
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


//------------------------------------------------------------------------------------------------------------------------

void setup() 
{
  turnOffWatchdogTimer();
  Serial.begin(9600);
  Serial.println(F("Test"));
  
// Set pinMode:
   DDRD |= B00010000; // Set Clock output
   DDRD |= B10000000; // Set Temp&Press output
   DDRC |= B00000100; // Set GPS MOSFET as output



  // Set Clock power pin (OUTPUT, HIGH)
  PORTD |= B00010000; // Set high
  
  delay(10);
  //EEPROM.update(98, 0); // Temporary in order to get time every boot.
  
  if (!EEPROM.read(98)){
    // We have not started the sender before, we need to set the time
    updateClock(1);
    EEPROM.update(98, 1);
  }

  Serial.println(F("The GPS is now off"));
  delay(3000);
  Serial.println(F("The GPS will now be turned on"));
  delay(50);
  
  // GPS Mosfet
  //PORTC |= B00000100; // Set to high
  // getPosition();
  //PORTC &= B11111011; // Set to low
  //Serial.println(F("The GPS is now off again"));

  updateClock(1);
  
  initializePacketNum();

  DateTime timeNow = RTC.now();
  EEPROM.update(800, timeNow.day());      // Indicating last date of reboot

  TimeAlarm.setWakeUpPeriod(0, 1, 0);
  // Set alarm the next 10-minute:
  // Sender 1 will start up 7 seconds early, 5 seconds for start and reading sensor input
  TimeAlarm.setAlarm1(1, 2*(SENDER_ADDRESS-2));
  printAlarm();

  Serial.print(F("Time: "));
  Serial.println(TimeAlarm.getTimeStamp());
  Serial.println(F("Sender on"));
  delay(100);
  goToSafeSleep();
}

//------------------------------------------------------------------------------------------------------------------------

void loop() 
{    

// Set pinMode wakeUp:
  DDRD |= B00010000; // Set Clock output
  DDRD |= B10000000; // Set Temp&Press output
  DDRC |= B00000100; // Set GPS MOSFET as output

// Set pinMode sleep:


  
  PORTD |= B00010000; // Set Clock high (output)
  
  delay(50);
  initializeAlarm();
  printAlarm();
  Serial.print(F("Woke up at time :"));
  Serial.println(TimeAlarm.getTime());
  delay(2000);
  
  uint8_t data[21]; // Uint8_t [2] can hold values in range 0-65'536
  // get() functions return long int. Data requires 15 bytes.
  fillLongIntToPos((long int) packageNum, 1, 0, data); // Adding packet number and adds sensor data to data.
  
  
  PORTD |= B10000000; // Set Temp&Press high (output)
  initializeTempAndPressure();
  fillLongIntToPos(getTemperature(), 2, 1, data); // Temp data (2 bytes)
  DDRD &= B01111111; // Set input
  PORTD &= B11111111; // Set pullup (not needed, just to show)

  
  PORTD |= B10000000; // Set high
  initializeTempAndPressure();
  fillLongIntToPos(getPressure(), 3, 5, data);  // Pressure data (3 bytes)
  DDRD &= B01111111; // Set input
  PORTD &= B11111111; // Set pullup (not needed, just to show)

  DDRD |= B01000000; // Set output
  PORTD |= B01000000; // Set high
  initializeHumidity();
  fillLongIntToPos(getHumidity(), 2, 3, data);  // Humidity data (2 byte)
  DDRD &= B10111111; // Set input
  PORTD &= B11111111; // Set pullup (not needed, just to show)
  
  initializeWaterPressureSensor();

  // Set Depth MOSFET and Multiplexer as Output
  DDRC |= B00000010; // Set MOSFET output
  DDRD |= B00100000; // Set Multiplexer output
  // Set Depth MOSFET and Multiplexer high
  PORTC |= B00000010; // Set MOSFET high
  PORTD |= B00100000; // Set Multiplexer high
  Serial.println(F("Going to get depth"));
  fillLongIntToPos(getDepth(), 3, 8, data);     // Depth data  (3 bytes)
  PORTC &= B11111101; // Set MOSFET low
  PORTD &= B11011111; // Set Multiplexer low
  Serial.println(F("Should have gotten depth"));
  
  fillLongIntToPos(getTime(), 4, 11, data);     // Time data (4 bytes)
  fillPositionData(data);                       // Position data (3 + 3 byte)       
                                           
                                            
  /*
  Serial.print(F("\nMessage generated, with number: "));
  Serial.println((int)data[0]); // Printing the first part of the message
  */
  
  // Set LoRa MOSFET pin
  DDRC |= B00001000; // Set output
  PORTC |= B00001000; // Set high
  delay(500);
  
  initializeLoRa();
  // Send newly generated data
  if (!manager.sendtoWait(data, sizeof(data), RECEIVER_ADDRESS))
  {     
    Serial.println(F("Send unsuccessful")); 
    storeInEEPROM(data, sizeof(data));         
  }
  
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
    getPosition();
    EEPROM.update(801, timeNow.month());
  }

  // Set LoRa MOSFET pin
  DDRC |= B00001000; // Set output
  PORTC &= B11110111; // Set low


  // Set Clock power pin (OUTPUT, LOW)
  DDRD |= B00010000; // Set output
  PORTD &= B11101111; // Set low
  Serial.println(F("Going to sleep at the bottom"));

  goToSafeSleep();
  Serial.println(F("Woke up at the bottom"));
  
}

