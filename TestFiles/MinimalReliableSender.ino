

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
// Don't put this on the stack:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
uint8_t memory[4][24]; // Memory that can store 4 packages of size 24
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
  for (int i = 0; i< 4; i++){
    packageMemory[i] = -1;
    memory[i][1] = (char)0; // Because first bit is packageNum
  }
}


void loop() {   
  uint8_t data[] = "Hello from the sender!"; 
  uint8_t numberedData[sizeof(packageNum) + sizeof(data)];
  addPackageNum(&numberedData[0], &data[0]);
  
  Serial.println("\nMessage generated: ");
  Serial.print((int)numberedData[0]); // Printing the first part of the message
  Serial.println((char*)&numberedData[1]); // Printing the text value
  
  if (!manager.sendtoWait(numberedData, sizeof(numberedData), RECEIVER_ADDRESS)){     
    // Send unsuccessful       
    writeToMemory(numberedData, sizeof(numberedData));   
    printFullDataMessage();   
  }
  sendFromMemory(); 
 

  // Listening for message for timeout ms
  int timeout = 2000;
  uint8_t bufLen = sizeof(buf);
  uint8_t from;
  
  // recv function returns buf with the data, 
  // bufLen with the data length,
  // from has the value of the sender
  // Receiving multiple times because of repeater, even uninteresting packages deserves an ack. 
  bool receiving = true; 
  while (receiving){
    clearReceivedData(buf, &bufLen, &from);
    if (manager.recvfromAckTimeout(buf, &bufLen, timeout, &from)){
      // Successfully received a message
      if (!packageInMemory((int)buf[0])){
        // This is a new message, we are interested
        updatePackageMemory((int)buf[0]);
        printReceived(&buf[0], from);
      }
      else{Serial.println("Received duplicate");};      
    }
    else receiving = false; // timeout
  }
  updatePackageNum();
  
  delay(6000);
}
void addPackageNum(uint8_t* result, uint8_t* input){
  result[0] = packageNum;
  for (int i=0; i< sizeof(input); i++){
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


