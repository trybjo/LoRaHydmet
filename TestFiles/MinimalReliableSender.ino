
/*
 * Pinout from Arduino to peripherals:
 * GND  -> Humidity sensor pin 3 (from left, seen from side with holes)
 * 5V   -> Humidity sensor pin 1 (from left, seen from side with holes) + Temp&Press VIN + LoRa VIN
 * 
 * A0   -> UNIK 500 (see sketch on github for connection)
 * 
 * D2   -> LoRa G0
 * D3   -> LoRa RST
 * 
 * D5   -> Rwmp&Press SDI (aka. MOSI)
 * D6   -> Temp&Press SDO (aka. MISO)
 * D7   -> Temp&Press SCK
 * D8   -> Temp&Press CS
 * 
 * D10  -> LoRa CS
 * D11  -> Temp&Press SDI + LoRa MOSI
 * D12  -> Temp&Press SDO + LoRa MISO
 * D13  -> Temp&Press SCK + LoRa SCK
 * 
 * 
 * SDA  -> Humidity sensor pin 2 (from left, seen from side with holes)
 * SCL  -> Humidity sensor pin 4 (from left, seen from side with holes)
 */
// Define pins for chip select, reset and interrut.
#define LoRa_CS 10
#define LoRa_RST 3
#define LoRa_INT 2

 
// Libraries for LoRa communication
#include <RHReliableDatagram.h>
#include <LoRa.h>
#include <RH_RF95.h>

// SPI library easy use of SPI. 
// MISO, MOIS and SCK hare handled automatically by the SPI-library. Chip select (CS)/Slave select (SS) must be set manually.
// CS/SS pin can be any digital pin. The slave communicates with master when its CS pin is low (usually...).
#include <SPI.h>

// Library to use I2C communucation.
#include <Wire.h>

// Library that some of Adafruit's sensors uses.
#include <Adafruit_Sensor.h>


#define SENDER_ADDRESS 1
#define RECEIVER_ADDRESS 2

// Set frequency [MHz]. The LoRa chip used here can operate at 868 MHz.
#define LoRa_FREQ 868.0

// Singleton instance of the radio driver.
RH_RF95 lora(LoRa_CS, LoRa_INT);

// Class to manage message delivery and receipt,using the lora declared above
RHReliableDatagram manager(lora, SENDER_ADDRESS);

// Variables for packet handling
uint8_t packageNum; 
// Don't put this on the stack:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
uint8_t memory[4][24]; // Memory that can store 4 packages of size 24
uint8_t packageMemory[4]; 
uint8_t packageMemoryPointer;

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

// Temperature and air pressure

// Library used for temperature and air pressure readings from the BMP280 chip.
#include <Adafruit_BMP280.h>

// Define, MOSI, MISO, SCK and chip celect pin.
#define BMP_MOSI 5
#define BMP_MISO 6
#define BMP_SCK 7
#define TempAndPressure_CS 8

// Set the CS pin.
Adafruit_BMP280 BMP(TempAndPressure_CS, BMP_MOSI, BMP_MISO, BMP_SCK);



// Humidity sensor

// Library used by the AM2329 sensor.
#include "Adafruit_AM2320.h"

// Instance for the humidity sensor.
Adafruit_AM2320 am2320 = Adafruit_AM2320();



// Water level senor

// The water level sensor don't need any preparation
// BUT THE MEASURING RANGE MUST BE FIGURED OUT. THE RESISTOR VALUE AND THEREFORE THE MAPPING VALUES MUST BE CHOSEN THEREAFTER.
// Higher resistor value -> higher resolution but lower measuring range.
// The microcontroller can be damaged if the resistor value is too high and the sensor is too deep in the water.
// Max depth -> 20mA. Max voltage to microcontroller = 5V. V=RI -> R=250 ohm for max depth. Higher resistor values can be used if only lower water levels are needed.


void setup() {
  Serial.begin(9600);
  while (!Serial) ; // Wait for serial port to be available
//  if (!manager.init())
//  // Init failed
//  delay(1000);
  initializeLoRa();
  initializeTempAndPressure();
  initializeHumidity();
  
  
  packageNum = 0;
  packageMemoryPointer = 0;
  for (int i = 0; i< 4; i++){
    packageMemory[i] = -1;
    memory[i][1] = (char)0; // Because first bit is packageNum
  }
}

void fillLongIntToPos(long int inValue, int requiredSize, int startingPos, uint8_t* outValue){
  uint8_t * _tempValue = (uint8_t *)&inValue;
  for (int i = 0; i < requiredSize; i++){
    outValue[startingPos + i] = _tempValue[i];
  }
}

void loop() {   

  // Testing data measurements
  uint8_t totalData[17];

  // Depth data taking 2 bytes
  long int depth = getDepth();
  // Humidity data taking 1 byte
  long int humidity = getHumidity();
  // Temp data taking 2 bytes
  long int temp = getTemp();
  // Pressure data taking 2 bytes
  long int pressure = getPressure();
  // Position takes 6 + 6 digits = 12 digits
  // 3 bytes + 3 bytes
  // 71.000 35.000
//  long int Longditude  = getLonditude();
//  long int Latitude = getLatitude();
  // Time + date taking 4 bytes? 

  fillLongIntToPos(depth, 2, 0, totalData);
  fillLongIntToPos(humidity, 1, 2, totalData);
  // All data requiering 17 bytes of data
  Serial.println(depth);
  Serial.println(humidity);
  uint8_t *vp = (uint8_t *)&depth;

  Serial.println("Depth and humidity: ");
  Serial.println((long int)totalData[0]); 
  Serial.println((long int)totalData[1]); //*256);
  Serial.println((long int)totalData[2]);
  
//  uint8_t a[4];
//  a[0] = vp[0];
//  a[1] = vp[1];
//  a[2] = vp[2];
//  a[3] = vp[3];

  // Uint8_t [2] can hold values in range 0-65'536
//  long int result = (long int)vp[0]+ (long int)vp[1]*256 + (long int)vp[2]*256*256 ;
//  Serial.println(result);
//  Serial.println(vp[0]);
//  Serial.println(vp[1]);
//  Serial.println(vp[2]);
  

  

  
  uint8_t data[] = "Hello from the sender!"; 
  uint8_t numberedData[sizeof(packageNum) + sizeof(data)];
  addPackageNum(&numberedData[0], &data[0], sizeof(data));
  
  Serial.println("\nMessage generated: ");
  Serial.print((int)numberedData[0]); // Printing the first part of the message
  Serial.println((char*)&numberedData[1]); // Printing the text value
  
  if (!manager.sendtoWait(numberedData, sizeof(numberedData), RECEIVER_ADDRESS)){     
    // Send unsuccessful       
    writeToMemory(numberedData, sizeof(numberedData));   
    printFullDataMessage();   
  }
  sendFromMemory(); 

  uint8_t bufLen = sizeof(buf);
  uint8_t from;
  
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
        printReceived(&buf[0], from);
      }
      else{Serial.println("Received duplicate");};      
    }
    else receiving = false;
  }
  updatePackageNum();  
  delay(6000);
}

//////////////// Initializing functions ////////////////
////////////////////////////////////////////////////////

// Initialize humidity sensor. Sets pins automatically.
void initializeHumidity()
{
  am2320.begin();
}

// Temperature and air pressure
// Initialize BMP. Sets all necessary pinModes.
void initializeTempAndPressure()
{
  BMP.begin();
}

// Initialize LoRa.
void initializeLoRa()
{
  //Set pinMode
  pinMode(LoRa_RST, OUTPUT);

  // Manual reset of LoRa module
  digitalWrite(LoRa_RST, HIGH); delay(100);
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

// Returns the humidity [%].
long int getHumidity()
{
  return 130;
//  return 12120;
//  return am2320.readHumidity();
}

// Returns the depth of the sensor [mm]. 
// The mapping values must be calibrated for the chosen resistor value.
long int getDepth()
{
  //return 65535;
  return 55000;
//  return floatMap(analogRead(A0), 411,647, 0,213);
}

// Returns temperature [*C].
long int getTemp()
{
  return 23230;
//  return BMP.readTemperature();
}

// Returns pressure [Pa].
long int getPressure()
{
  return 45450;
//  return BMP.readPressure();
}



//////////////////////////////////////////////////////

// A mapping function that can return floating (decimal) values.
float floatMap(float x, float in_min, float in_max, float out_min, float out_max)
{
return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void addPackageNum(uint8_t* result, uint8_t* input, int sizeOfInput){
  result[0] = packageNum;
  for (int i=0; i< sizeOfInput; i++){
    result[i+1] = input[i];
  }
}

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

void sendFromMemory(){ // trying to send all elements of memory
  for (int i = 0; i <4; i++){ 
    // Iterating over the 4 memory positions    
    if (memory[i][1] != (char)0){   
      // Only send data that exists     
      if (!manager.sendtoWait(&memory[i][0], sizeof(memory[i]), RECEIVER_ADDRESS)){ // Trying to send from memory
        Serial.println("Send from memory unsuccessful");
      }
      else{
        Serial.print("Send from memory success with size: ");
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
// Returning 0 if memory is full
int findEmptyMemory(){    
  for (int i = 0; i < 4; i++){ // Assuming memory has size of 4
    if (memory[i][1] == 0){
      Serial.print("Found empty memory at position: ");
      Serial.println(i);      
      return i;
    }
  }
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


//////////////////// Print functions ////////////////////
void printFullDataMessage(){
  for (int i = 0; i < 4 ; i++){
    if (memory[i][1] != (char)0){
      Serial.print("Memory: ");
      Serial.print(i);
      Serial.print(" : ");
      Serial.print((int)memory[i][0]); 
      Serial.println((char*)&memory[i][1]); 
    }    
  }
  Serial.println("");
}

void printReceived(uint8_t* message, uint8_t from){
  Serial.print("Received a message: ");
  Serial.print((int)message[0]);
  Serial.println((char*)&message[1]);
  Serial.print("Sender of the message was : 0x");
  Serial.println(from, HEX);
}

//////////// These functions take ~ 80 bytes /////////////


