
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

void loop() {
  if (manager.available()){
    Serial.println();
    uint8_t bufLen = sizeof(buf); // Needs to be here to convert to uint8_t
    uint8_t from;
    bool receiveSuccess = false;
    if (!manager.recvfromAck(buf, &bufLen, &from)){ //Function changes value of buf, bufLen and from
      Serial.println("Receive unsuccessful, was message not for me?");
      receiveSuccess = false;
    }
    else{
      receiveSuccess = true;
      if (!packageInMemory((int)buf[0])){
        updatePackageMemory((int)buf[0]);
        Serial.print("Received message from 0x");
        Serial.print(from, HEX);
        Serial.print(": ");
        Serial.print((int)buf[0]);
        Serial.println((char*)&buf[1]);
      }
      else Serial.println("Received duplicate");          
    }
    // After receiving one message, there may be more incoming
    bool receiving = true; // 1 when receiving
    int timeout = 700; // ms
    while (receiving){
      if (!manager.recvfromAckTimeout(buf, &bufLen, timeout, &from)){ // Listening for incoming messages 
       Serial.println("Assuming no more messages for this time");
       receiving = false;
      }
      else{
       if (!packageInMemory((int)buf[0])){
        updatePackageMemory((int)buf[0]);
        Serial.print("Received message from 0x");
        Serial.print(from, HEX);
        Serial.print(": ");
        Serial.print((int)buf[0]);
        Serial.println((char*)&buf[1]);   
       }
       else 
         Serial.println("Received duplicate");
      }
    } 

    // After successful receive(s), send a message: 
    if (receiveSuccess){
      uint8_t data[] = "And hello back to you";
      uint8_t new_temp[sizeof(packageNum) + sizeof(data)];
      new_temp[0] = packageNum; // Adding packet number
      for(int i = 0; i<sizeof(data); i++){
        new_temp[i+1] = data[i];
      }
      
      if (!manager.sendtoWait(new_temp, sizeof(new_temp), from)){
        Serial.println("Sending failed");
      }
      else {
        Serial.print("Sending successful: ");
        Serial.print((int)new_temp[0]);
        Serial.println((char*)&new_temp[1]);
        
        if(packageNum >=7){packageNum = 0;}
        else packageNum ++;
      }
    }
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

