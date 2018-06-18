

#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <SPI.h>


#define SENDER_ADDRESS 1
#define RECEIVER_ADDRESS 2
#define RF95_FREQ 868.0

RH_RF95 lora;     // Instanciate the LoRA driver

// Class to manage message delivery and receipt,using the lora declared above
RHReliableDatagram manager(lora, SENDER_ADDRESS);
uint8_t packageNum; 
int memoryPointer;
// Don't put this on the stack:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
uint8_t memory[4][24]; // Memory that can store 4 packages of size 24
int memoryList[4]; // Indicates what memory is filled



void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while (!Serial) ; // Wait for serial port to be available
  if (!manager.init())
  Serial.println("init failed");
  delay(1000);
  packageNum = 0;
  memoryPointer = 0;
  for (int i = 0; i< 4; i++){
    memoryList[0] = 0;
  }
  Serial.println("Setup completed");
}

void loop() {  
  // Sending message: 
  uint8_t data[] = "Hello from the sender!"; 
  uint8_t new_temp[sizeof(packageNum) + sizeof(data)];
  new_temp[0] = packageNum;
  for (int i=0; i< sizeof(data); i++){
    new_temp[i+1] = data[i];
  }
  Serial.println();
  sendFromMemory(memory, memoryList);
  Serial.println("Message generated: ");
  Serial.print((int)new_temp[0]); // Printing the first part of the message
  Serial.println((char*)&new_temp[1]); // Printing the text value
  
  if (!manager.sendtoWait(data, sizeof(data), RECEIVER_ADDRESS)){ // Sending message and waiting for reply
    Serial.println("Send unsuccessful");
    
    // Testing if the memory has space for the new data
    if (memoryPointer >= 4){ // This is highest legal value, need to look for empty memory
      printFullDataMessage();
      memoryPointer = findEmptyMemory();
    }  
    writeToMemory(new_temp, sizeof(new_temp)); 
    memoryPointer ++;
  }
  else Serial.println("Send successful");

  // Listening for message for timeout ms
  int timeout = 2000;
  uint8_t bufLen = sizeof(buf);
  uint8_t from;
  // This function returns buf with the data, bufLen with the dataSize and from has the value of the sender. 
  if (!manager.recvfromAckTimeout(buf, &bufLen, timeout, &from)){ // Listening for incoming messages 
    Serial.println("Receive failed, check if server is online");
  }
  else{
    Serial.println("Received a message from the server: ");
    Serial.println((char*)buf);
    Serial.print("Sender of the message was : 0x");
    Serial.println(from, HEX);
  }  
  if (packageNum >= 7){ packageNum = 0;} // counting package numbers 0-7 to try to use 3 bit.
  packageNum ++;
  delay(6000);
}

// uint8_t memory[][24]
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
      
      if (!manager.sendtoWait(&memory[i][0], sizeof(&memory[i][0]), RECEIVER_ADDRESS)){ // Trying to send from memory
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

void writeToMemory(uint8_t* message, int messageLength){ // This function does not work, trouble getting the full message
  Serial.print("Storing message in memory position: ");
  Serial.println(memoryPointer);
  for(int i = 0; i < messageLength; i++){
    memory[memoryPointer][i] = message[i];
  }
  memoryList[memoryPointer] = 1; // Indicating that the position is filled
}


void printFullDataMessage(){
  Serial.println("memoryPointer > 4, potential overwrite");
  for (int i = 0; i < memoryPointer ; i++){
    Serial.print("For iterator value: ");
    Serial.print(i);
    Serial.print(" : ");
    Serial.print((int)memory[i][0]); // The number that is in front of the message
    Serial.println((char*)&memory[i][1]); // The text in the message
  }
  Serial.println("");
}
