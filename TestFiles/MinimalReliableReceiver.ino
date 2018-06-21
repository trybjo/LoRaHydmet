
#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <SPI.h>

#define SENDER_ADDRESS 1
#define RECEIVER_ADDRESS 2

RH_RF95 driver;
RHReliableDatagram manager(driver, RECEIVER_ADDRESS); // Instanciate this object with address.
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
uint8_t packageMemory[4];
uint8_t packageMemoryPointer;
uint8_t packageNum;



void setup() {
  Serial.begin(9600);
  while (!Serial) ; // Wait for serial port to be available
  if (!manager.init())
  Serial.println("init failed");
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
      printReceived(&buf[0], from[0]);
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


void loop() {
  if (manager.available()){
    Serial.println();
    uint8_t bufLen = sizeof(buf); // Needs to be here to convert to uint8_t
    uint8_t from;
    bool duplicate;
    bool receiveSuccess = receive(&duplicate, buf, &bufLen, &from, -1);

    // After receiving one message, there may be more incoming
    bool receiving = true; // 1 when receiving
    int timeout = 1000; // ms
    while (receiving){
      receiving = receive(&duplicate, buf, &bufLen, &from, timeout);
    }

    // After successful receive(s), send a message: 
    if (receiveSuccess){
      uint8_t data[] = "And hello back to you";
      uint8_t numberedResponse[sizeof(packageNum) + sizeof(data)];
      addPackageNum(&numberedResponse[0], &data[0], sizeof(data));
      
      if (!manager.sendtoWait(numberedResponse, sizeof(numberedResponse), from)){
        Serial.println("Sending failed");
      }
      else {
        Serial.print("Sending successful: ");
        Serial.print((int)numberedResponse[0]);
        Serial.println((char*)&numberedResponse[1]);
        updatePackageNum();
      }
    }
  }
}
void updatePackageNum(){
  if (packageNum >= 7){ packageNum = 0;} 
  else packageNum ++;
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

