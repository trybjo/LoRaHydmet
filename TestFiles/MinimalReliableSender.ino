

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
uint8_t packageMemory[4];
uint8_t packageMemoryPointer;



void setup() {
  Serial.begin(9600);
  while (!Serial) ; // Wait for serial port to be available
  if (!manager.init())
  // Init failed
  delay(1000);
  packageNum = 0;
  packageMemoryPointer = 0;
  memoryPointer = 0;  
  for (int i = 0; i< 4; i++){
    packageMemory[i] = -1;
    memoryList[i] = 0;
  }
  // Setup complete
}

void loop() {   
  uint8_t data[] = "Hello from the sender!"; 
  uint8_t new_temp[sizeof(packageNum) + sizeof(data)];
  new_temp[0] = packageNum; // Adding package number
  for (int i=0; i< sizeof(data); i++){
    new_temp[i+1] = data[i];
  }
  Serial.println();
  
  Serial.println("Message generated: ");
  Serial.print((int)new_temp[0]); // Printing the first part of the message
  Serial.println((char*)&new_temp[1]); // Printing the text value
  
  if (!manager.sendtoWait(new_temp, sizeof(new_temp), RECEIVER_ADDRESS)){ // Sending message and waiting for reply
    
    // Send unsuccessful    
    // Testing if the memory has space for the new data
    memoryPointer = findEmptyMemory();    
    writeToMemory(new_temp, sizeof(new_temp));   
    printFullDataMessage();   
  }
  else{
    Serial.print("Sending size: ");
    Serial.println(sizeof(new_temp));
  }
  sendFromMemory();// Lasts because of setup in repeater 
 

  // Listening for message for timeout ms
  int timeout = 2000;
  uint8_t bufLen = sizeof(buf);
  uint8_t from;
  
  // recv function returns buf with the data, 
  // bufLen with the data length,
  // from has the value of the sender
  // Receiving multiple times because of repeater, even uninteresting packages deserves an ack. 
  int receivingBit = 1; 
  while (receivingBit){
    clearReceivedData(buf, &bufLen, &from);
    if (manager.recvfromAckTimeout(buf, &bufLen, timeout, &from)){
      // Successfully received a message
      if (!packageInMemory((int)buf[0])){
        // This is a new message, we are interested
        updatePackageMemory((int)buf[0]);
        Serial.print("Received a message: ");
        Serial.print((int)buf[0]);
        Serial.println((char*)&buf[1]);
        Serial.print("Sender of the message was : 0x");
        Serial.println(from, HEX);
      }
      else{Serial.println("Received duplicate");};
      
    }
    else receivingBit =0; // timeout
  }
  
  if (packageNum >= 7){ packageNum = 0;} // counting package numbers 0-7 to try to use 3 bit.
  else packageNum ++;
  delay(6000);
}

void clearReceivedData(uint8_t* buf,uint8_t* bufLen, uint8_t* from){
  buf[0] = (char)0; // Setting termination bit
  bufLen = 0;
  from = (char)0;
}

// Returning position of unused memory
// Returning 0 if memory is full
int findEmptyMemory(){ 
  
//  if (memoryPointer < 3){
//    // We can add 1, and still have a legal value.
//    return memoryPointer ++;
//  }
  // else: 
  for (int i = 0; i < 4; i++){ // Assuming memory has size of 4
    if (memory[i][1] == 0){
      Serial.print("Found empty memory at position: ");
      Serial.println(i);      
      return i;
    }
  }
  return 0; // No memory is full, overwriting should start at pos 0
}

int sendFromMemory(){// uint8_t memory1[][24], int memoryList1[]){ // sending all elements of memory, returns 1 for success
  for (int i = 0; i <4; i++){ 
    // Iterating over the 4 memory positions
    if (memoryList[i] == 1){ 
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
//  memoryPointer ++;
}


void printFullDataMessage(){
  for (int i = 0; i <= memoryPointer ; i++){
    Serial.print("Memory: ");
    Serial.print(i);
    Serial.print(" : ");
    Serial.print((int)memory[i][0]); // The number that is in front of the message
    Serial.println((char*)&memory[i][1]); // The text in the message
  }
  Serial.println("");
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

