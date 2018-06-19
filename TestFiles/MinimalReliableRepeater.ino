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

uint8_t memoryReceiver[4][24]; // Memory that can store 4 packages of size 24
int memoryListReceiver[4]; // Indicates what memory is filled
int memoryPointerReceiver;
int packageMemoryFromReceiver[4]; // Storing the package number of messages from the receiver
uint8_t packageMemoryFromReceiverPointer; // Points at next place to write memory


uint8_t memorySender[4][24];
int memoryListSender[4];
int memoryPointerSender; // Pointer to next memory to be written
int packageMemoryFromSender[4]; // Storing the package number of messages from the sender 
uint8_t packageMemoryFromSenderPointer;



void setup() {
  Serial.begin(9600);
  while (!Serial) ; // Wait for serial port to be available
  if (!manager.init())
  Serial.println("init failed");
  delay(1000);
  packageNum = 0;
  memoryPointerReceiver = 0;
  memoryPointerSender = 0;
  packageMemoryFromReceiverPointer = 0;
  packageMemoryFromSenderPointer = 0;
  
  for (int i = 0; i< 4; i++){
    memoryListReceiver[i] = 0;
    memoryListSender[i] = 0;
    packageMemoryFromSender[i] = -1; // Initating as invalid package number
    packageMemoryFromReceiver[i] = -1;
  }
  Serial.println("Setup repeater completed");
}

void loop() {
  if (manager.available()){
    Serial.println();
    uint8_t bufLen = sizeof(buf); // Needs to be here to convert to uint8_t
    uint8_t from;  // From becomes author of the message
    
//    if (!manager.recvfromAck(buf, &bufLen, &from)){ //Function changes value of buf, bufLen and from
//      // Receive not successful
//      Serial.println("Receive unsuccessful");      
//    }
//    else{ 
//      // Receive success
//      if (!packageInMemory((int)buf[0], &from)){
//        // New message, this is interesting
//        updatePackageMemory((int)buf[0], &from);
//        Serial.print("Received message from 0x");
//        Serial.print(from, HEX);
//        Serial.println(": ");
//        Serial.print((int)buf[0]);
//        Serial.println((char*)&buf[1]);  
//        if (!forwardMessage(buf, bufLen, from)){ // Pretends to be original sender when forwardning
//          // The forwarding was not successful, need to write to memory
//          updatePointer(from);        
//          writeToMemory(buf, bufLen, from); 
//          printFullDataMessage();
//        } 
//      }
//      // else, we received a duplicate              
//    }

    // After one receive, we want to listen for more messages
    bool receiving = true;
    int timeout = 1000; // ms
    while(receiving){  
      clearReceivedData(buf, &bufLen, &from);
      if (!manager.recvfromAckTimeout(buf, &bufLen, &from)){ //Function changes value of buf, bufLen and from
        // Receive not successful
        Serial.println("Receive unsuccessful, assuming no more messages.");
        receiving = false;      
      }
      else{ 
        // Receive success
        if (!packageInMemory((int)buf[0], &from)){
          updatePackageMemory((int)buf[0], &from);
          Serial.print("Received message from 0x");
          Serial.print(from, HEX);
          Serial.println(": ");
          Serial.print((int)buf[0]);
          Serial.println((char*)&buf[1]);  
          if (!forwardMessage(buf, bufLen, from)){ // Pretends to be original sender when forwardning
            // The forwarding was not successful, need to write to memory
            updatePointer(from);        
            writeToMemory(buf, bufLen, from); 
            printFullDataMessage();
          }        
        }
        // else, we received a duplicate
                 
      }
    }
    sendFromMemory(memorySender, memoryListSender, SENDER_ADDRESS, RECEIVER_ADDRESS);
    sendFromMemory(memoryReceiver, memoryListReceiver, RECEIVER_ADDRESS, SENDER_ADDRESS);
  }
}

void clearReceivedData(uint8_t* buf,uint8_t* bufLen, uint8_t* from){
  buf[0] = (char)0; // Setting termination bit
  bufLen = 0;
  from = (char)0;
}
void updatePackageMemory(int package, uint8_t from){
  if (from == SENDER_ADDRESS){
    packageMemoryFromSender[packageMemoryFromSenderPointer] = package;
    if (packageMemoryFromSenderPointer < 3){
      // The pointer can legally increase
      packageMemoryFromSenderPointer++;
    }
    else packageMemoryFromSenderPointer = 0;
  }
  else if (from == RECEIVER_ADDRESS){
    packageMemoryFromReceiver[packageMemoryFromReceiverPointer] = package;
    if (packageMemoryFromReceiverPointer < 3){
      // The pointer can legally increase
      packageMemoryFromReceiverPointer++;
    }
    else packageMemoryFromReceiverPointer = 0;
  }
  
}

// Returns 1 if the package is in memory
int packageInMemory(int package, uint8_t from){
  if (from == SENDER_ADDRESS){
    for (int i = 0; i < sizeof(packageMemoryFromSender); i++){
      // Iteates over memory
      if (packageMemoryFromSender[i] == package){
        return 1;
      }
    }
  }
  else if (from == RECEIVER_ADDRESS){
    for (int i = 0; i < sizeof(packageMemoryFromReceiver); i++){
      if (packageMemoryFromReceiver[i] == package){
        return 1;
      }
    }
  }  
  return 0;
}

// Pretends to be the original sender, and sends to the original destination
int forwardMessage(uint8_t* buf, int bufLen, uint8_t from){
  
  if(from == SENDER_ADDRESS){ // the originator of the message is the sender    
    
    //sendtoWaitRepeater(buf,len, destinationAddress, mySenderAddress)
    if (!manager.sendtoWaitRepeater(buf, bufLen, RECEIVER_ADDRESS, SENDER_ADDRESS)){
      Serial.println("Forward unnsuccessful");
      
      return 0;
      
    }
    else{
      Serial.println("Forward successful");
      return 1;
    }
  }
  else if (from == RECEIVER_ADDRESS){ // The originator of the message is the receiver
    if(!manager.sendtoWaitRepeater(buf, bufLen, SENDER_ADDRESS, RECEIVER_ADDRESS)){
      Serial.println("Forward unnsuccessful");
      return 0;
    }
    else{
      Serial.println("Forward successful");
      return 1;
    }
  }
}

// Adds 1 to corresponding pointer if value < 4. 
// If not, finds empty position
// If not, sets pointer to 0
int updatePointer(uint8_t from){ // Returning 5 if memory is full
  if (from == SENDER_ADDRESS){
    if (memoryPointerSender < 4){
      // Pointer inside legal boundaries after ++
      // memoryPointerSender ++; 
      return 1;
    }
    else{
      for (int i = 0; i < 4; i++){ // Assuming memory has size of 4
        if (memorySender[i][1] == 0){
          Serial.print("Found empty memory at position: ");
          Serial.println(i);      
          memoryPointerSender = i;
          return 1;
        }      
      }
      memoryPointerSender = 0;
      return 0;
    }
  }
  else if (from == RECEIVER_ADDRESS){
    if (memoryPointerReceiver < 4){
      // Pointer inside legal boundaries after ++
      // memoryPointerReceiver ++; 
      return 1;
    }
    else{
      for (int i = 0; i < 4; i++){ // Assuming memory has size of 4
        if (memoryReceiver[i][1] == 0){
          Serial.print("Found empty memory at position: ");
          Serial.println(i);      
          memoryPointerReceiver = i;
          return 1;
        }         
      }
      memoryPointerReceiver = 0; //  
      return 0;  
    }
  return 0; // No memory is full, overwriting should start at pos 0
  }
}

// sending all elements of memory, returns 1 for success
int sendFromMemory(uint8_t memory[][24], int memoryList[], uint8_t author, uint8_t endDest){ 
  for (int i = 0; i <4; i++){ // Iterating over the 4 memory positions
    if (memoryList[i] == 1){ 
      // Don't try to send empty memory
      
      if (!manager.sendtoWaitRepeater(&memory[i][0], sizeof(memory[i]), endDest, author)){ // Trying to send from memory
        // Not success
      }
      else{
        Serial.print("Send from memory success");
        deleteFromMemory(i, author);        
      }       
    }        
  }
}

void deleteFromMemory(int deletePosition, uint8_t author){
  if (author == SENDER_ADDRESS){
    for(int i = 0; i < 24; i++){ // The full pre-defined memory size
      memorySender[deletePosition][i] = 0;
    }
    memoryListSender[deletePosition] = 0; // Indicating the memory is emptied
  }
  else if (author == RECEIVER_ADDRESS){
    for(int i = 0; i < 24; i++){
      memoryReceiver[deletePosition][i] = 0;
    }
    memoryListReceiver[deletePosition] = 0; 
  }  
}

// Writes to corresponding memory, updates memoryList and memoryPointer
void writeToMemory(uint8_t* message, int messageLength, uint8_t author){ // This function does not work, trouble getting the full message
  if (author == SENDER_ADDRESS){ // We write to the sender memory
    for(int i = 0; i < messageLength; i++){    
     memorySender[memoryPointerSender][i] = message[i];
    }
    memoryListSender[memoryPointerSender] = 1; // Indicating this position is filled
    memoryPointerSender ++;    
  }
  else if (author == RECEIVER_ADDRESS){ // We write to receiver memory
    for(int i = 0; i < messageLength; i++){
      memoryReceiver[memoryPointerReceiver][i] = message[i];
    }
    memoryListReceiver[memoryPointerReceiver] = 1; // Indicating this position is filled
    memoryPointerReceiver ++;
  }  
}


void printFullDataMessage(){
  Serial.println("Memory:");
  for (int i = 0; i < 4 ; i++){
    if (memoryListSender[i]){
      Serial.print("For iterator value: ");
      Serial.print(i);
      Serial.print(" : ");
      Serial.print((int)memorySender[i][0]); // The number that is in front of the message
      Serial.println((char*)&memorySender[i][1]); // The text in the message
    }    
  }
  Serial.println("");
}

