
#include "memoryRepeater.h"
#include <Arduino.h>

#define SENDER1_ADDRESS 1
#define SENDER2_ADDRESS 2
#define SENDER3_ADDRESS 3
#define SENDER4_ADDRESS 4
#define SENDER5_ADDRESS 5
#define SENDER6_ADDRESS 6

#define RECEIVER_ADDRESS 7
#define REPEATER_ADDRESS 8

// Dynamic variables:
uint8_t packageNumberPointer1 = 0;
uint8_t packageNumberPointer2 = 0;
uint8_t packageNumberPointer3 = 0;
uint8_t packageNumberPointer4 = 0;
uint8_t packageNumberPointer5 = 0;
uint8_t packageNumberPointer6 = 0;
uint8_t packageNumberPointerReceiver = 0; // Points at next place to write packet number to memory

void loadDynamicVariables();

void writeMessageToMemory(uint8_t* message, int messageLength, uint8_t from){
  int packageNum = message[0];
  for(int i = 0; i < messageLength; i++){
    EEPROM.update(100*packageNum + 15*(from-1) + i, message[i]);
  }
}


bool initializeMemory(){ 
  EEPROM.update(90,0); // Force memory whipe
  if(EEPROM.read(90) == 1){
    // Position 90 is 1 if the processor has been startet before
    // The processor is waking up from a reboot
    loadDynamicVariables();
    return false;
  }
  else{    
    // Deleting memory from all senders
    for (int i = 0; i < 8; i++){
      // i representing packet number
      for (int j = 1; j < 7; j++){
        // j representing sender address
        deleteFromMemory(j, i);
      }
    }
    // Removing all packet numbers and setting packet pointers 0
    for (int j = 1; j < 7; j++){
      // j representing each sender, could be up to six
      EEPROM.update(100*j+ 94, 0);
      for (int i = 0; i < 4; i++){
        // j representing the four packet number positions
        EEPROM.update(100*j + 90 + i, 8);
        // 8 is invalid packet number
      }
    }
    EEPROM.update(90,1); 
    // Indicating that the processor has been initiated
    return true;
  }
}

void deleteFromMemory(uint8_t author, int packetNum){
  // Element 11 of each message is 0, indicating empty memory
  // If not empty, memory position 11 contains month
  if (0 < author && author < 7 && 0 <= packetNum && packetNum  < 8){
    EEPROM.update(100*packetNum + 15*(author-1) + 11, 0);
  }
}

void updatePackageNumberPointer(uint8_t &memPointer){
  if (memPointer < 3){
    memPointer ++;
  }
  else memPointer = 0;
}

void updatePackageNumberMemory(int package, int from, uint8_t &memPointer){
  EEPROM.update(from*100+90+memPointer, package);
  updatePackageNumberPointer(memPointer);
}


void updatePackageNumberMemory(int package, uint8_t from){
  switch(from){
    case 1:
      updatePackageNumberMemory(package, from, packageNumberPointer1);
      break;
    case 2:
      updatePackageNumberMemory(package, from, packageNumberPointer2);
      break;
    case 3:
      updatePackageNumberMemory(package, from, packageNumberPointer3);
      break;
    case 4:
      updatePackageNumberMemory(package, from, packageNumberPointer4);
      break;
    case 5:
      updatePackageNumberMemory(package, from, packageNumberPointer5);
      break;
    case 6:
      updatePackageNumberMemory(package, from, packageNumberPointer6);
      break;
    case 7:
      updatePackageNumberMemory(package, from, packageNumberPointerReceiver);
      break;
  }
}




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
  packageNumberPointer5 = valueLimit(EEPROM.read(100*SENDER5_ADDRESS + 94));
  packageNumberPointer6 = valueLimit(EEPROM.read(100*SENDER6_ADDRESS + 94));
  packageNumberPointerReceiver = valueLimit(EEPROM.read(100*RECEIVER_ADDRESS + 94));
}

void writeDynamicVariablesToMemory(){
  // Pointers for the package number memory 
  EEPROM.update(100*SENDER1_ADDRESS + 94, packageNumberPointer1);
  EEPROM.update(100*SENDER2_ADDRESS + 94, packageNumberPointer2);
  EEPROM.update(100*SENDER3_ADDRESS + 94, packageNumberPointer3);
  EEPROM.update(100*SENDER4_ADDRESS + 94, packageNumberPointer4);
  EEPROM.update(100*SENDER5_ADDRESS + 94, packageNumberPointer5);
  EEPROM.update(100*SENDER6_ADDRESS + 94, packageNumberPointer6);
  EEPROM.update(100*RECEIVER_ADDRESS + 94, packageNumberPointerReceiver);
}


bool packageInMemory(int packageNum, uint8_t from){
  for (int i = 0; i<4; i++){
    if (EEPROM.read(from*100+90+i) == packageNum){
      return true;
    }
  }
  return false;
}