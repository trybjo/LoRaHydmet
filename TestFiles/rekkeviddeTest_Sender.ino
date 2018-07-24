#include "TinyGPS++.h"
#include "SoftwareSerial.h"

SoftwareSerial serial_connection(7, 6); //RX=pin 6, TX=pin 7
TinyGPSPlus gps;//This is the GPS object that will pretty much do all the grunt work with the NMEA data


// Define pins for chip select, reset and interrut.
#define LoRa_CS 10
#define LoRa_RST 3
#define LoRa_INT 2

 
// Libraries for LoRa communication
#include <RHReliableDatagram.h>
#include <LoRa.h>
#include <RH_RF95.h>

#define RFM95_CS 10
#define RFM95_RST 3
#define RFM95_INT 2

// SPI library easy use of SPI. 
// MISO, MOIS and SCK hare handled automatically by the SPI-library. Chip select (CS)/Slave select (SS) must be set manually.
// CS/SS pin can be any digital pin. The slave communicates with master when its CS pin is low (usually...).
#include <SPI.h>

#define SENDER_ADDRESS 1
#define RECEIVER_ADDRESS 2

// Set frequency [MHz]. The LoRa chip used here can operate at 868 MHz.
#define LoRa_FREQ 870.1

// Singleton instance of the radio driver.
RH_RF95 lora(LoRa_CS, LoRa_INT);

// Class to manage message delivery and receipt,using the lora declared above
RHReliableDatagram manager(lora, SENDER_ADDRESS);

// Variables for packet handling
uint8_t packageNum; 
// Don't put this on the stack:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
uint8_t memory[4][15]; // Memory that can store 4 packages of size 14
uint8_t memoryPointer;
uint8_t packageMemory[4]; 
uint8_t packageMemoryPointer;

// Set spreading factor, 6 ~ 12. 
// LoRa is a Chirp Spread Spectrum (CSS) modulation. This setting determines the number of chirps per bit of data.
// At the highest value (SF 12), the link has higher range and is more robust, but the data rate is lowered significantly. 
// The lowest value (SF 6) is a special case designed for the highest possible data rate. In this mode however, the length of the packet has to be known in advance. Also, CRC check is disabled.
#define spreadingFactor 12

// Set bandwidth, option: 7800,10400,15600,20800,31250,41700,62500,125000,250000,500000 [Hz].
// Lower BandWidth for longer distance. Semtech recommends to keep it above or equal to 62.5 kHz to be compatible with the clock speed.
#define signalBandwidth 625E2

// Set coding rate, 5 ~ 8 (values are actually 4/x).
// LoRa modem always performs forward error correction. The amount of these error checks is expressed by the coding rate. 
// The lowest value (CR 4/5) results in higher data rate and less robust link, the highest value (CR 4/8) provides more robust link at the expense of lowering data rate.
#define codingRateDenominator 8


void setup() {
  
  Serial.begin(9600);//This opens up communications to the Serial monitor in the Arduino IDE
  serial_connection.begin(9600);//This opens up communications to the GPS
  delay(100);
  Serial.println("GPS Start");//Just show to the monitor that the sketch has started
  Serial.println("Test 1");

  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  // manual reset  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);
  initializeLoRa();
  
  packageNum = 0;
  packageMemoryPointer = 0;
  memoryPointer = 0;
  for (int i = 0; i< 4; i++){
    packageMemory[i] = -1;
    memory[i][1] = (char)0; // Because first bit is packageNum
  }
}


void loop() {   

  long int latitude = 111111;
  long int longitude = 111111;
  uint8_t data[9];
    // Adding packet number
    fillLongIntToPos((long int) packageNum, 1, 0, data);

while(serial_connection.available())//While there are characters to come from the GPS
  {
    gps.encode(serial_connection.read());//This feeds the serial NMEA data into the library one char at a time
  }
  if(gps.location.isUpdated())//This will pretty much be fired all the time anyway but will at least reduce it to only after a package of NMEA data comes in
  {

   latitude = (gps.location.lat() *10000);
   longitude = (gps.location.lng() *10000);

    //Get the latest info from the gps object which it derived from the data sent by the GPS unit
    Serial.println("Satellite Count:");
    Serial.println(gps.satellites.value());

fillLongIntToPos(latitude, 4, 1, data);
    fillLongIntToPos(longitude, 4, 5, data);

    Serial.print(F("\nMessage generated, with number: "));
    Serial.println((int)data[0]); // Printing the first part of the message
    
    sendFromMemory();
  
    // Send newly generated data
    if (!manager.sendtoWait(data, sizeof(data), RECEIVER_ADDRESS)){     
      // Send unsuccessful       
      writeToMemory(data, sizeof(data));       
      printFullDataMessage();   
    }
    Serial.print(F("RSSI [dBm] of last sent message  : "));
    Serial.println((int)lora.lastRssi());
     
    uint8_t bufLen = sizeof(buf);
    uint8_t from;

    /*
    // Receiving multiple times because of repeater, even uninteresting packages deserves an ack.
    // timeout = 2000 ms
    bool receiving = true; 
    while (receiving){
      clearReceivedData(buf, &bufLen, &from);
      if (manager.recvfromAckTimeout(buf, &bufLen, 2000, &from)){
        // Successfully received a message
        if (!packageInMemory((int)buf[0])){
          // This is a new message, we are interested
          updatePackageMemory((int)buf[0]);
        }
        else{Serial.println(F("Received duplicate"));};      
      }
      else receiving = false;
    }
    */
    
    updatePackageNum();  
  }


  
}









//////////////// Initializing functions ////////////////
////////////////////////////////////////////////////////

// Initialize LoRa.
void initializeLoRa()
{
  //Set pinMode
  pinMode(LoRa_RST, OUTPUT);

  // Manual reset of LoRa module
  digitalWrite(LoRa_RST, HIGH); 
  delay(100);
  digitalWrite(LoRa_RST, LOW);
  delay(10);
  digitalWrite(LoRa_RST, HIGH);
  delay(10);

  // Initialize both LoRa and Reliable Datagram
  if (!manager.init())
  // Init failed
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
////////////////////////////////////////////////////////

//////////////// Reading measurements ////////////////
//////////////////////////////////////////////////////

// counting package numbers 0-7 to try to use 3 bit at some time.
void updatePackageNum(){
  if (packageNum >= 7){ packageNum = 0;} 
  else packageNum ++;
}

void clearReceivedData(uint8_t* buf,uint8_t* bufLen, uint8_t* from){
  buf[0] = (char)0; // Setting termination bit
  buf[1] = (char)0;
  bufLen = 0;
  from = (char)0;
}

// Iterating over memory, trying to send all of them
void sendFromMemory(){ 
  for (int i = 0; i <4; i++){ 
    // Iterating over the 4 memory positions    
    if (memory[i][1] != (char)0){   
      // Only send data that exists     
      if (!manager.sendtoWait(&memory[i][0], sizeof(memory[i]), RECEIVER_ADDRESS)){ // Trying to send from memory
        Serial.println(F("Send from memory unsuccessful"));
      }
      else{
        Serial.print(F("Send from memory success with size: "));
        Serial.println(sizeof(memory[i]));      
        deleteFromMemory(i);        
      }       
    }        
  }
}

void deleteFromMemory(int deletePosition){
  memory[deletePosition][0] = (char)0; // Setting first element as termination bit
  memory[deletePosition][1] = (char)0;
}

// Returning position of unused memory
// Returning values from 0-3 if memory is full, incrementing starting at last filled position
int findEmptyMemory(){    
  // The for-loop looks for empty memory
  for (int i = 0; i < 4; i++){ // Assuming memory has size of 4
    if (memory[i][1] == 0){
      Serial.print(F("Found empty memory at position: "));
      Serial.println(i); 
      memoryPointer = i;     
      return i;
    }
  }
  // Memory is full
  if (memoryPointer < 3){ 
    // We can increment legally
    memoryPointer ++;
    return memoryPointer;
  }
  memoryPointer = 0;
  return 0; // No memory is empty, overwriting should start at pos 0
}

void writeToMemory(uint8_t* message, int messageLength){ 
  int _emptyMemory = findEmptyMemory();
  for(int i = 0; i < messageLength; i++){
    memory[_emptyMemory][i] = message[i];
  }
}



// Stores package number of new packages incoming
void updatePackageMemory(int package){
  packageMemory[packageMemoryPointer] = package;
  if (packageMemoryPointer < 3){
    packageMemoryPointer++;
  }
  else packageMemoryPointer = 0;
}

// Returns 1 if the package is in memory
int packageInMemory(int package){
  for (int i = 0; i < 4; i++){
    if (packageMemory[i] == package){
      return 1;
    }
  }
  return 0;
}

void printFullDataMessage(){
  for (int i = 0; i < 4 ; i++){
    if (memory[i][1] != (char)0){
      Serial.print(F("Memory: "));
      Serial.print(i);
      Serial.print(" : ");
      Serial.println((int)memory[i][0]); 
    }    
  }
  Serial.println("");
}

void printReceived(uint8_t* message, uint8_t from){
  Serial.print(F("Received a message: "));
  Serial.print((int)message[0]);
  Serial.println((char*)&message[1]);
  Serial.print(F("Sender of the message was : 0x"));
  Serial.println(from, HEX);
}




void fillLongIntToPos(long int inValue, int requiredSize, int startingPos, uint8_t* outValue){
  uint8_t * _tempValue = (uint8_t *)&inValue;
  for (int i = 0; i < requiredSize; i++){
    outValue[startingPos + i] = _tempValue[i];
  }
}
