#include <RH_Repeater_ReliableDatagram.h>
#include <LoRa.h>
#include <RH_RF95.h>
#include <SPI.h>
#include <EEPROM.h>
#include "TinyGPS++.h"
#include "RTClibExtended.h"
#include <Adafruit_Sensor.h>
#include <Wire.h>

// Define pins for chip select, reset and interrupt.
#define LoRa_CS 10
#define LoRa_RST 4
#define LoRa_INT 3

// Defining addresses
#define SENDER1_ADDRESS 1
#define SENDER2_ADDRESS 2
#define SENDER3_ADDRESS 3
#define SENDER4_ADDRESS 4

#define RECEIVER_ADDRESS 5
#define REPEATER_ADDRESS 6

#define RF95_FREQ 868.0
#define MESSAGE_LEN 15

// Set spreading factor, 6 ~ 12. 
// LoRa is a Chirp Spread Spectrum (CSS) modulation. This setting determines the number of chirps per bit of data.
// At the highest value (SF 12), the link has higher range and is more robust, but the data rate is lowered significantly. 
// The lowest value (SF 6) is a special case designed for the highest possible data rate. In this mode however, the length of the packet has to be known in advance. Also, CRC check is disabled.
#define spreadingFactor 12

// Set bandwidth, option: 7800,10400,15600,20800,31250,41700,62500,125000,250000,500000 [Hz].
// Lower BandWidth for longer distance. Semtech recommends to keep it above or equal to 62.5 kHz to be compatible with the clock speed.
#define signalBandwidth 125E3

// Set coding rate, 5 ~ 8 (values are actually 4/x).
// LoRa modem always performs forward error correction. The amount of these error checks is expressed by the coding rate. 
// The lowest value (CR 4/5) results in higher data rate and less robust link, the highest value (CR 4/8) provides more robust link at the expense of lowering data rate.
#define codingRateDenominator 8


RTC_DS3231 RTC;                                               // Instanciate the external clock
TinyGPSPlus gps;                                              // Instanciate the GPS
RH_RF95 lora(LoRa_CS, LoRa_INT);                              // Instanciate the LoRA driver
RH_Repeater_ReliableDatagram manager(lora, REPEATER_ADDRESS); // Instanciate the reliable sender

// Using pin 2 as wake up pin
const int wakeUpPin = 2;

// Dynamic memory for receiving messages:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

// Dynamic variables:
uint8_t packageNumberPointer1 = 0;
uint8_t packageNumberPointer2 = 0;
uint8_t packageNumberPointer3 = 0;
uint8_t packageNumberPointer4 = 0;
uint8_t packageNumberPointerReceiver = 0; // Points at next place to write packet number to memory


void setup() {
  Serial.begin(115200);
  while (!Serial) ; // Wait for serial port to be available
  if (!manager.init())
  Serial.println(F("init failed"));
  delay(1000); 
  initializeLoRa();
  initializeMemory();
  /*
  initializeTimer();
  Serial.println(getTime());
  initializeAlarm();
  initializeMemory();
  // hh, mm, ss
  setWakeUpPeriod(0, 1, 2);
  initWakeUpTime(14, 45, 0);
  setAlarm1();
  */
  Serial.println(F("Setup repeater completed"));
  delay(10);

}


void loop() {
  if (manager.available()){
        
    uint8_t bufLen = sizeof(buf); // Needs to be here to convert to uint8_t
    uint8_t from;  // from becomes author of the message
    uint8_t to; // to becomes the intended receiver of the message
    bool duplicate;

    // receive(bool, message, bufLen, from, timeout);
    bool receiveSuccess = myReceive(duplicate, buf, &bufLen, &from, &to, 6000);
    if (receiveSuccess && !duplicate){
      if (!forwardMessage(buf, bufLen, from, to)){ 
        // The forwarding was not successful, need to write to memory
        if (from != RECEIVER_ADDRESS){
          // We only store messages from the senders             
          writeMessageToMemory(buf, bufLen, from); 
          printMemory(from);
        }         
      }
      else Serial.println(F("Forward success"));
    }
    Serial.print(F("Package number pointer: "));
    Serial.println(packageNumberPointer1);
    sendFromMemory();
  }
}
/*
void setWakeUpPeriod(int wHour, int wMinute, int wSec){
  EEPROM.update(43, wHour);
  EEPROM.update(44, wMinute);
  EEPROM.update(45, wSec);
}
void initWakeUpTime(int wHour, int wMinute, int wSec){
  EEPROM.update(46, wHour);
  EEPROM.update(47, wMinute);
  EEPROM.update(48, wSec);
}
void updateWakeUpTime(){
  // Updates the current wake-up time by adding period
  EEPROM.update(46, EEPROM.read(46)+ EEPROM.read(43)); // Hour
  EEPROM.update(47, EEPROM.read(47)+ EEPROM.read(44)); // Minute
  EEPROM.update(48, EEPROM.read(48)+ EEPROM.read(45)); // Sec
}
void initializeAlarm(){
  pinMode(wakeUpPin, INPUT);
  
  //clear any pending alarms
  RTC.armAlarm(1, false);
  RTC.clearAlarm(1);
  RTC.alarmInterrupt(1, false);
  RTC.armAlarm(2, false);
  RTC.clearAlarm(2);
  RTC.alarmInterrupt(2, false);

  // Set interrupt mode
  RTC.writeSqwPinMode(DS3231_OFF);
}

void wakeUp(){  
  // Just a handler for the pin interrupt.
}
void goToSleep(){
  // digitalPinToInterrupt(3)
  attachInterrupt(0, wakeUp, FALLING);
  // Disable ADC
  ADCSRA &= ~(1<<7);
  
  // Go to deep sleep
  SMCR |= (1<<2); // Power down mode
  SMCR |= 1; // Enable sleep
  
  // BOD disable
  MCUCR |= (3<<5); // Set both BODS and BODSE at the same time
  MCUCR = (MCUCR & ~(1<<5)) | (1<<6); // Then set the BODS bit and clear the BODSE bit at the same time
  __asm__ __volatile__("sleep"); // In line assembler sleep execute instruction
  detachInterrupt(0);
}

void initializeTimer(){
  Wire.begin();
  if (! RTC.begin()) {
    Serial.println(F("Couldn't find rtc"));
    while (1);
  } 
  RTC.adjust(DateTime(__DATE__, __TIME__));
  
  /*
  pinMode(8, OUTPUT);
  digitalWrite(8, HIGH);
  Serial.println("GPS Start");//Just show to the monitor that the sketch has started
  Wire.begin();
  if (! RTC.begin()) {
    Serial.println(F("Couldn't find rtc"));
    while (1);
  } 
  
  while (!gps.location.isUpdated()){
    // Waiting for location to get updated
    while (Serial.available()){
      gps.encode(Serial.read());
    }
  }
  while(!(gps.date.year() && gps.date.month() &&gps.date.day())){
    // Wait until all values are filled
    while (Serial.available()){
      gps.encode(Serial.read());
    }
  }  
  uint16_t yy = gps.date.year();
  uint8_t mm = gps.date.month();
  uint8_t dd = gps.date.day();
  uint8_t hh = gps.time.hour();
  uint8_t Min = gps.time.minute();
  uint8_t sec = gps.time.second();
  RTC.adjust(DateTime(yy, mm, dd, hh, Min, sec));
  digitalWrite(8, LOW);
  Serial.println("From the clock: ");
  delay(200);
  Serial.println(getTime());
  delay(2000);
}
  */
long int getTime(){
  DateTime timeNow = RTC.now(); 
  // year = 2018, year%100 = 18
  long int timeInt = (long)timeNow.month() * 100000000 + (long)timeNow.day()*1000000 +((long)timeNow.year()%100)*10000;
  timeInt += (long)timeNow.hour()*100 + (long) timeNow.minute();
  return timeInt; 
}
/*
void setAlarm1(){
  int hh = EEPROM.read(46);
  int mm = EEPROM.read(47);
  int ss = EEPROM.read(48);
  Serial.print(F("Setting alarm to: mm, hh, ss: "));
  Serial.print(mm);
  Serial.print(", ");
  Serial.print(hh);
  Serial.print(", ");
  Serial.println(ss);
  RTC.setAlarm(ALM1_MATCH_HOURS, mm, hh, ss);   //set your wake-up time here
  RTC.alarmInterrupt(1, true);
}*/

// Initialize LoRa.
void initializeLoRa()
{
  //Set pinMode
  pinMode(LoRa_RST, OUTPUT);
  pinMode(LoRa_INT, OUTPUT);

  // Manual reset of LoRa module
  digitalWrite(LoRa_RST, HIGH); 
  delay(100);
  digitalWrite(LoRa_RST, LOW);
  delay(10);
  digitalWrite(LoRa_RST, HIGH);
  delay(10);
  digitalWrite(LoRa_INT, HIGH);
  delay(100);
  digitalWrite(LoRa_INT, LOW);
  delay(10);
  digitalWrite(LoRa_INT, HIGH);
  delay(10);
  pinMode(LoRa_INT, OUTPUT);
  
  // Initialize both LoRa and Reliable Datagram
  if (!manager.init())
  // Init failed
  delay(1000);

  // Initialize LoRa.
  //  lora.init();

  // Set frequency.
  lora.setFrequency(RF95_FREQ);
  
  // Set transmitter power, value from 7-23 (23 = 20 dBm). 13 dBm is default. 
  // Can go above this using PA_BOOST, which RFM9x has.
  lora.setTxPower(23);

  // Set spreding factor, bandwidth and coding rate. 
  // See LoRa #define in top of code for description and usage.
//  LoRa.setSpreadingFactor(spreadingFactor); // This causes failure
  LoRa.setSignalBandwidth(signalBandwidth);
  LoRa.setCodingRate4(codingRateDenominator);
}


void stopAlarm(){
  RTC.armAlarm(1, false);
  RTC.clearAlarm(1);
  RTC.alarmInterrupt(1, false);
  delay(20);
}


void initializeMemory(){ 
  EEPROM.update(90,0); 
  if(EEPROM.read(90) == 1){
    // Position 42 is 1 if the processor has been startet before
    // The processor is waking up from a reboot
    //loadDynamicVariables();
    
  }
  else{
    // Element 11 of each message is 0, indicating empty memory
    Serial.println(F("First time, initializing values to zero"));
    // Deleting memory from all senders
    for (int i = 0; i<7; i++){
      deleteFromMemory(SENDER1_ADDRESS, i);
      deleteFromMemory(SENDER2_ADDRESS, i);
      deleteFromMemory(SENDER3_ADDRESS, i);
      deleteFromMemory(SENDER4_ADDRESS, i);
    }

    // Removing all packet numbers and setting packet pointers 0
    for (int j = 0; j < 6; j++){
      // j representing each sender, could be up to six
      EEPROM.update(100*j+ 94, 0);
      for (int i = 0; i < 4; i++){
        EEPROM.update(100*j + 90 + i, 8);
        // 8 is invalid packet number
      }
    }
    EEPROM.update(90,1);
  }
}

void writeMessageToMemory(uint8_t* message, int messageLength, uint8_t from){
  int packageNum = message[0];
  for(int i = 0; i < messageLength; i++){
    EEPROM.update(100*packageNum + 15*(from-1) + i, message[i]);
  }
  // OBS OBS OBS OBS : MIDLERTIDIG
  // EEPROM.update(100*packageNum + 15*(from-1) + 11, 7);
  // _____________________________
}

// Return 'value' if in range 0-3, else return 0
uint8_t valueLimit(uint8_t value){
  if (value <= 3 && value >=0){
    return value;
  }
  return 0;
}

void loadDynamicVariables(){
  // Pointers for the package number memory
  packageNumberPointer1 = valueLimit(EEPROM.read(100*SENDER1_ADDRESS + 94));
  packageNumberPointer2 = valueLimit(EEPROM.read(100*SENDER2_ADDRESS + 94));
  packageNumberPointer3 = valueLimit(EEPROM.read(100*SENDER3_ADDRESS + 94));
  packageNumberPointer4 = valueLimit(EEPROM.read(100*SENDER4_ADDRESS + 94));
  packageNumberPointerReceiver = valueLimit(EEPROM.read(100*RECEIVER_ADDRESS + 94));
}

void writeDynamicVariablesToMemory(){
  // Pointers for the package number memory 
  EEPROM.update(100*SENDER1_ADDRESS + 94, packageNumberPointer1);
  EEPROM.update(100*SENDER2_ADDRESS + 94, packageNumberPointer2);
  EEPROM.update(100*SENDER3_ADDRESS + 94, packageNumberPointer3);
  EEPROM.update(100*SENDER4_ADDRESS + 94, packageNumberPointer4);
  EEPROM.update(100*RECEIVER_ADDRESS + 94, packageNumberPointerReceiver);
}


// Receives message and detects duplicates
bool myReceive(bool &duplicate, uint8_t* message, uint8_t* bufLen, uint8_t* from, uint8_t* to, int timeout){
  bool success;
  if (manager.recvfromAckTimeout(buf, bufLen, timeout, from, to)){
    // Receive successful
    success = true;
    if (!packageInMemory((int)buf[0], *from)){
      // New message, we are interested
      Serial.println(F("We are interested"));
      duplicate = false;
      updatePackageNumberMemory((int)buf[0], *from);
      printReceived(&buf[0], from[0]);
    }
    else {
      Serial.println(F("Received duplicate"));
      duplicate = true;
    }
  }
  else{
    duplicate = false;
    success = false;
  }
  return success;
}

// Stores the package number to memory corresponding to the sender
// Updates the pointer to memory position containing latest package number
void updatePackageNumberMemory(int package, uint8_t from){
  switch(from){
    case 1:
      updatePackageMemory(package, from, packageNumberPointer1);
      break;
    case 2:
      updatePackageMemory(package, from, packageNumberPointer2);
      break;
    case 3:
      updatePackageMemory(package, from, packageNumberPointer3);
      break;
    case 4:
      updatePackageMemory(package, from, packageNumberPointer4);
      break;
    case 5:
      updatePackageMemory(package, from, packageNumberPointerReceiver);
      break;
  }
}

void updatePackageMemory(int package, int from, uint8_t &memPointer){
  EEPROM.update(from*100+90+memPointer, package);
  updatePackagePointer(memPointer);
}

void updatePackagePointer(uint8_t &memPointer){
  if (memPointer < 3){
    memPointer ++;
  }
  else memPointer = 0;
}

// Returns true if the package is in memory
bool packageInMemory(int package, uint8_t from){
  Serial.print(F("Looking for: "));
  Serial.print(package);
  Serial.print(F(" among: "));
  for (int i = 0; i<4; i++){
    Serial.print(EEPROM.read(100*from+90+i));
    Serial.print(F(", "));
    if (EEPROM.read(from*100+90+i) == package){
      return true;
    }
  }
  Serial.println();
  return false;
}


// Pretends to be the original sender, and sends to the original destination
bool forwardMessage(uint8_t* buf, int bufLen, uint8_t from, uint8_t to){
  // The following is supposed to replace the rest. 
  return manager.sendtoWaitRepeater(buf, bufLen, to, from);
}

void sendFromMemory(){
  sendFromMemory(SENDER1_ADDRESS, RECEIVER_ADDRESS);
  sendFromMemory(SENDER2_ADDRESS, RECEIVER_ADDRESS);
  sendFromMemory(SENDER3_ADDRESS, RECEIVER_ADDRESS);
  sendFromMemory(SENDER4_ADDRESS, RECEIVER_ADDRESS);
}
// Trying to send all elements of memory from given author
void sendFromMemory(uint8_t author, uint8_t endDest){
  for (int i = 0; i <7; i++){
    if (EEPROM.read(100*i + 15*(author-1) + 11) != 0){
      // Don't try to send empty memory 
      uint8_t buf [MESSAGE_LEN];
      // Storing from memory to uint8_t*
      for (int j = 0; j < MESSAGE_LEN; j++){
        buf[j] = EEPROM.read(100*i + MESSAGE_LEN*(author-1) + j);
      }
      if (manager.sendtoWaitRepeater(&buf[0], MESSAGE_LEN, endDest, author)){ // Trying to send from memory
        Serial.println(F("Send from memory success"));
        deleteFromMemory(i, author);          
      }
      else{
        // Not success  
        Serial.println(F("Send from memory not a success"));  
      }      
    }
  }
}

void deleteFromMemory(uint8_t author, int packetNum){
  EEPROM.update(100*packetNum + 15*(author-1) + 11, 0);
}


void printPackageNumberMemory(int from){
  Serial.print(F("Package memory from "));
  Serial.print(from);
  Serial.println(F(": "));
  for (int i = 0; i < 4; i++){
    Serial.println(EEPROM.read(100*from+90+i)); 
  }
}

void printReceived(uint8_t* message, uint8_t from){
  Serial.print(F("Received message from: "));
  Serial.print(from, HEX);
  Serial.print(F(" : "));
  Serial.println((int)message[0]);  
}
void printMemory(int from){
  Serial.print(F("Memory from "));
  Serial.print(from);
  Serial.println(F(":"));
  
  for (int i = 0; i < 7 ; i++){
    if (EEPROM.read(100*i + 15*(from-1) + 11) != 0){
      Serial.print(F("For iterator value: "));
      Serial.print(i);
      Serial.print(F(" : "));
      Serial.println(EEPROM.read(100*i + 15*(from-1))); // The number that is in front of the message
    }    
  }
  Serial.println();
}

