
#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <SPI.h>
#include <LoRa.h>

#define SENDER_ADDRESS 1
#define RECEIVER_ADDRESS 2

#define LoRa_CS 10
#define LoRa_RST 3
#define LoRa_INT 2
#define LoRa_FREQ 871.0

RH_RF95 driver(LoRa_CS, LoRa_INT);
RHReliableDatagram manager(driver, RECEIVER_ADDRESS); // Instanciate this object with address.
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
uint8_t packageMemory[4];
uint8_t packageMemoryPointer;
uint8_t packageNum;

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



void setup() {
  Serial.begin(9600);
  delay(100);
  
  digitalWrite(LoRa_RST, LOW);
  delay(10);
  pinMode(LoRa_RST, OUTPUT);
  delay(10);
  digitalWrite(LoRa_RST, HIGH);
  while (!Serial) ; // Wait for serial port to be available
  if (!manager.init())
  Serial.println("init failed");

  driver.setFrequency(LoRa_FREQ);
  driver.setTxPower(13, false);
  LoRa.setSignalBandwidth(signalBandwidth);
  LoRa.setCodingRate4(codingRateDenominator);
  
  for (int i = 0; i < 4; i++){
    packageMemory[i] = -1;
  }
  packageNum = 0;
  packageMemoryPointer = 0;
}

// The function calls receive function with or without timeout
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
    if (!packageInMemory((int)buf[0])){
      *duplicate = false;
      updatePackageMemory((int)buf[0]);
//      printReceived(&buf[0], from[0]);
    }
    else {
      Serial.println("Received duplicate");
      *duplicate = true;
    }
    return true;
  }
  else{
    *duplicate = false;
    return false;
  }
}

WriteDataToSerial(uint8_t* message){
  // Signal strength:
    Serial.print(driver.lastRssi(), DEC);
    Serial.print(" , ");
    // Packet number:
    Serial.print(uint8PosToLongInt(message, 1, 0));
    Serial.print(" , ");
    // Temperature:    
    Serial.print(uint8PosToFloat(message, 2, 1, 2));
    Serial.print(" , ");
    // Humidity:
    Serial.print(uint8PosToFloat(message, 2, 3, 2));
    Serial.print(" , ");
    // Pressure:
    Serial.print(uint8PosToLongInt(message, 3, 5));
    Serial.print(" , ");
    // Debth:
    Serial.print(uint8PosToFloat(message, 3, 8, 2));
    Serial.print(" , ");
    // Time: 
    Serial.println(uint8PosToLongInt(message, 4, 11));
}


void loop() {
  if (manager.available()){
    // Serial.println();
    uint8_t bufLen = sizeof(buf); // Needs to be here to convert to uint8_t
    uint8_t from;
    bool duplicate;
    
    // We receive, and wait forever to receive it
    bool receiveSuccess = receive(&duplicate, buf, &bufLen, &from, -1);
    WriteDataToSerial(buf);
    // After receiving one message, there may be more incoming because of repeaters and buffers
    // We wait for 1 sec, between each receive before we give up 
    bool receiving = true;
    int timeout = 1000;
    while (receiving){
      receiving = receive(&duplicate, buf, &bufLen, &from, timeout);
      if receiving{
        WriteDataToSerial(buf);
      }
    }
    
      
    // After successful receive(s), send a message: 
    if (receiveSuccess){
      uint8_t data[] = "And hello back to you";
      uint8_t numberedResponse[sizeof(packageNum) + sizeof(data)];
      addPackageNum(&numberedResponse[0], &data[0], sizeof(data));
      
      if (!manager.sendtoWait(numberedResponse, sizeof(numberedResponse), from)){
        // Serial.println("Sending failed");
      }
      else {
        // Serial.print("Sending successful: ");
        // Serial.print((int)numberedResponse[0]);
        // Serial.println((char*)&numberedResponse[1]);
        updatePackageNum();
      }
    }
  }
}
void updatePackageNum(){
  if (packageNum >= 7){ packageNum = 0;} 
  else packageNum ++;
}

float uint8PosToFloat(uint8_t* input, int usedSize, int startPos, int decimals){
  long int _temp = 0;
  for (int i = startPos; i< startPos + usedSize; i++){
    _temp += (long int)input[i] * toPowerOf(256, i-startPos); 
  }
  return (float)_temp/toPowerOf(10, decimals);  
}

long int toPowerOf(int input, int power){
//  if (power == 0){
//    return 1;
//  }
//  else{
    long int temp = 1;
    for (int i = 0; i < power; i++){
      temp *= input;
    }
    return temp;
//  }  
}

long int uint8PosToLongInt(uint8_t* input, int usedSize, int startPos){
  long int _temp = 0;
  for (int i = startPos; i< startPos + usedSize; i++){
    _temp += (long int)input[i] * toPowerOf(256, i-startPos); 
  }
  return _temp;  
}

void addPackageNum(uint8_t* result, uint8_t* input, int sizeOfInput){
  result[0] = packageNum;
  for (int i=0; i< sizeOfInput; i++){
    result[i+1] = input[i];
  }
}

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

void printReceived(uint8_t* message, uint8_t from){
  Serial.print("Received message from: ");
  Serial.print(from, HEX);
  Serial.print(" : ");
  Serial.print((int)message[0]);
  Serial.println((char*)&message[1]);  
}
