#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include "systemConstants.h"
#include "TinyGPS++.h"


TinyGPSPlus gps;
RH_RF95 lora(LoRa_CS, LoRa_INT);

RHReliableDatagram manager(lora, RECEIVER_ADDRESS); // Instanciate this object with address.
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

void setup() {
  Serial.begin(9600);
  delay(100);
  
  while (!Serial) ; // Wait for serial port to be available
  initializeLoRa();
  delay(1000);
  initiateGPS();
  //Serial.println(F("Signal Strength, Packet number, Temp, Humidity, Pressure, Debth, Time"));
  Serial.println(F("Receiver on"));
}

void loop() {
  if (manager.available()){
    // Serial.println();
    uint8_t bufLen = sizeof(buf); // Needs to be here to convert to uint8_t
    uint8_t from;
    bool duplicate;
    
    // We receive, and wait forever to receive it
    receive(&duplicate, buf, &bufLen, &from, -1);
    
    writeDataToSerial(buf);
    double RLatF = 11.1111;
    double RLngF = 11.1111;
    
    while(Serial.available())//While there are characters to come from the GPS
    {
      gps.encode(Serial.read());//This feeds the serial NMEA data into the library one char at a time
    }
    RLatF = gps.location.lat();
    RLngF = gps.location.lng();
    
    Serial.print(F(", receiver pos: "));
    Serial.print(RLatF, 4);
    Serial.print(", ");
    Serial.println(RLngF, 4);
  }
}

// The function calls receive function with or without timeout
// Detects duplicates and update package memory
// message, bufLen and from are called by reference
// This means their values are updated after function call
bool receive(bool* duplicate, uint8_t* message, uint8_t* bufLen, uint8_t* from, int timeout){
  bool success;
  if (timeout == -1){    
     success = manager.recvfromAck(buf, bufLen, from);
  }
  else {
     success = manager.recvfromAckTimeout(buf, bufLen, timeout, from);
  }
  
  if (success){
    return true;
  }
  else{
    return false;
  }
}

void writeDataToSerial(uint8_t* message){
  //sPrintData(input, usedSize, startPos, decimals);
  
  // Signal strength dBm
  Serial.print((int)lora.lastRssi()); // THis one need to be chagned still
  Serial.print(F(","));
  
  // Packet number:
  sPrintData(message, 1, 0, 0);
  Serial.print(F(","));
  
  // Latitude:    
  sPrintData(message, 4, 1, 4);
  Serial.print(F(","));
  // Longditude:
  sPrintData(message, 4, 5, 4);
  //Serial.println();
}

// Decoding uint8_t* to integer or decimal values and writing to serial.
// When 'decimals' == 0, the integer value will be printed.
void sPrintData(uint8_t* input, int usedSize, int startPos, int decimals){
  long int _temp = 0;
  for (int i = startPos; i< startPos + usedSize; i++){
    _temp += (long int)input[i] * toPowerOf(256, i-startPos); 
  }
  if (!decimals){
    Serial.print(_temp);
  }
  else{
    long int tenPower = toPowerOf(10,decimals);
    Serial.print(_temp/tenPower); // Integer value
    Serial.print(F("."));
    Serial.print(_temp - tenPower*(_temp/tenPower)); // Decimals
  }
}

long int toPowerOf(int input, int power){
  long int temp = 1;
  for (int i = 0; i < power; i++){
    temp *= input;
  }
  return temp; 
}

void initiateGPS(){
  Serial.println("GPS Start");//Just show to the monitor that the sketch has started
  Wire.begin();
  int iterator = 0;
  while (iterator < 4){
    while(Serial.available())//While there are characters to come from the GPS
    {
      gps.encode(Serial.read());//This feeds the serial NMEA data into the library one char at a time
    }
    if(gps.location.isUpdated())//This will pretty much be fired all the time anyway but will at least reduce it to only after a package of NMEA data comes in
    {
      iterator ++;
    }
  }
  Serial.println(gps.time.minute());
}  

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
  Serial.println(F("Init failed"));
  delay(1000);

  // Initialize LoRa.
  //  lora.init();

  // Set frequency.
  lora.setFrequency(LoRa_FREQ);
  
  // Set transmitter power, value from 7-23 (23 = 20 dBm). 13 dBm is default. 
  // Can go above this using PA_BOOST, which RFM9x has.
  lora.setTxPower(23);

  // Set spreding factor, bandwidth and coding rate. 
  // See LoRa #define in top of code for description and usage.
//  LoRa.setSpreadingFactor(spreadingFactor); // This causes failure
  LoRa.setSignalBandwidth(signalBandwidth);
  LoRa.setCodingRate4(codingRateDenominator);
}

