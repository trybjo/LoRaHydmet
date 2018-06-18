#include <RH_Repeater_ReliableDatagram.h>
#include <RH_RF95.h>
#include <SPI.h>


#define SENDER_ADDRESS 1
#define RECEIVER_ADDRESS 2
#define REPEATER_ADDRESS 3
#define RF95_FREQ 868.0

RH_RF95 lora;     // Instanciate the LoRA driver

// Class to manage message delivery and receipt,using the lora declared above
RH_Repeater_ReliableDatagram manager(lora, REPEATER_ADDRESS);

uint8_t packageNum; 


// Don't put this on the stack:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
uint8_t memory[4][24]; // Memory that can store 4 packages of size 24
int memoryList[4]; // Indicates what memory is filled
int packageMemoryFromSender[4]; // Storing the package number of messages from the sender 
int memoryPointerSender;
int packageMemoryFromReceiver[4]; // Storing the package number of messages from the receiver
int memoryPointerReceiver;

void setup() {
  Serial.begin(9600);
  while (!Serial) ; // Wait for serial port to be available
  if (!manager.init())
  Serial.println("init failed");
  delay(1000);
  packageNum = 0;
  memoryPointer = 0;
  for (int i = 0; i< 4; i++){
    memoryList[i] = 0;
    packageMemoryFromSender[i] = -1;
    packageMemoryFromReceiver[i] = -1;
  }
  Serial.println("Setup completed");
}

void loop() {
  if (manager.available()){
    uint8_t bufLen = sizeof(buf); // Needs to be here to convert to uint8_t
    uint8_t from;    
    if (!manager.recvfromAck(buf, &bufLen, &from)){ //Function changes value of buf, bufLen and from
      Serial.println("Receive unsuccessful");
    }
    else{
      Serial.print("Received message from 0x");
      Serial.print(from, HEX);
      Serial.println(": ");
      Serial.print((int)buf[0]);
      Serial.println((char*)&buf[1]);  
      forwardMessage(buf, bufLen, from); // Pretends to be original sender when forwardning
               
    }
  }
}

void forwardMessage(uint8_t* buf, int bufLen, uint8_t from){
  if(from == SENDER_ADDRESS){ // the originator of the message is the sender
    
    //sendtoWaitRepeater(buf,len, destinationAddress, mySenderAddress)
    if (!manager.sendtoWaitRepeater(buf, bufLen, RECEIVER_ADDRESS, SENDER_ADDRESS)){
      Serial.println("Forward unnsuccessful");
    }
    else{
      Serial.println("Forward successful");
    }
    return 1;
  }
  else if (from == RECEIVER_ADDRESS){ // The originator of the message is the receiver
    if(!manager.sendtoWaitRepeater(buf, bufLen, SENDER_ADDRESS, RECEIVER_ADDRESS)){
      Serial.println("Forward unnsuccessful");
    }
    else{
      Serial.println("Forward successful");
    }
  }
}


int findEmptyMemory(){ // Returning 5 if memory is full
  for (int i = 0; i < 4; i++){ // Assuming memory has size of 4
    if (memory[i][1] == 0){
      Serial.print("Found empty memory at position: ");
      Serial.println(i);      
      return i;
    }
  }
  return 0; // No memory is full, overwriting should start at pos 0
}

int sendFromMemory(uint8_t memory[][24], int memoryList[]){ // sending all elements of memory, returns 1 for success
  for (int i = 0; i <4; i++){ // Iterating over the 4 memory positions
    if (memoryList[i] == 1){ // Don't try to send empty memory
      
      if (!manager.sendtoWaitRepeater(&memory[i][0], sizeof(&memory[i][0]), RECEIVER_ADDRESS, SENDER_ADDRESS)){ // Trying to send from memory
        Serial.println("Send from memory unsuccessul");
      }
      else{
        Serial.println("Send from memory success");
        deleteFromMemory(i);        
      }       
    }        
  }
}

void deleteFromMemory(int deletePosition){
  for(int i = 0; i < 24; i++){ // The full pre-defined memory size
    memory[deletePosition][i] = 0;
  }
  memoryList[deletePosition] = 0; // Indicating the memory is emptied
}

void writeToMemory(uint8_t* message, int messageLength, uint8_t author){ // This function does not work, trouble getting the full message
  if (author == SENDER_ADDRESS){
    for(int i = 0; i < messageLength; i++){    
    packageMemoryFromSender[memoryPointerSender][i] = message[i];
    }
    memoryListSender[memoryPointerSender] = 1 // Indicating this position is filled
    memoryPointerSender ++;    
  }
  else if (author == RECEIVER_ADDRESS){
    for(int i = 0; i < messageLength; i++){
      
    }
  }
  Serial.print("Storing message in memory position: ");
  Serial.println(memoryPointer);
  
  memoryList[memoryPointer] = 1; // Indicating that the position is filled
}

