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

int packageMemoryFromReceiver[4]; // Storing the package number of messages from the receiver
uint8_t packageMemoryFromReceiverPointer; // Points at next place to write memory


uint8_t memorySender[4][24];
int packageMemoryFromSender[4]; // Storing the package number of messages from the sender 
uint8_t packageMemoryFromSenderPointer;



void setup() {
  Serial.begin(9600);
  while (!Serial) ; // Wait for serial port to be available
  if (!manager.init())
  Serial.println(F("init failed"));
  delay(1000);
  packageNum = 0;
  packageMemoryFromReceiverPointer = 0;
  packageMemoryFromSenderPointer = 0;
  
  for (int i = 0; i< 4; i++){
    packageMemoryFromSender[i] = -1; // Initating as invalid package number
    packageMemoryFromReceiver[i] = -1;
    memorySender[i][1] = (char)0;
  }
  Serial.println(F("Setup repeater completed"));
}

void loop() {
  if (manager.available()){
    uint8_t bufLen = sizeof(buf); // Needs to be here to convert to uint8_t
    uint8_t from;  // from becomes author of the message
    uint8_t to; // to becomes the intended receiver of the message
    
    if (manager.recvfromAck(buf, &bufLen, &from &to)){ //Function changes value of buf, bufLen and from
      // Receive success
      if (!packageInMemory((int)buf[0], from)){
        // New message, this is interesting
        updatePackageMemory((int)buf[0], from);
        printReceived(&buf[0], from);
          
        if (!forwardMessage(buf, bufLen, from, to)){ // Pretends to be original sender when forwardning
          // The forwarding was not successful, need to write to memory
          if (from == SENDER_ADDRESS){
            // We only store messages from the sender             
            writeToMemory(buf, bufLen); //, from); 
            printMemory();
          }  
          else Serial.println(F("Forward to sender not successful"));        
        }
        else Serial.println(F("Success"));
        // else, forward success
      }
      // else, we received a duplicate              
    }
    sendFromMemory(memorySender, SENDER_ADDRESS, RECEIVER_ADDRESS);
  }
}

void updatePackageMemory(int package, uint8_t from){
  if (from == SENDER_ADDRESS){
    updatePackageMemory(&packageMemoryFromSender[0], &packageMemoryFromSenderPointer, package);
  }
  else if (from == RECEIVER_ADDRESS){
    updatePackageMemory(&packageMemoryFromReceiver[0], &packageMemoryFromReceiverPointer, package);
  }
}
void updatePackageMemory(int* packageMemory, uint8_t* memPointer, int package){
  packageMemory[*memPointer] = package;
  updatePointer(memPointer);
}

void updatePointer(uint8_t* memPointer){
  if (memPointer[0] < 3){
    memPointer[0] ++;
  }
  else memPointer[0] = 0;
}

 
// Returns 1 if the package is in memory
int packageInMemory(int package, uint8_t from){
  if (from == SENDER_ADDRESS){
    return packageInMemory(&packageMemoryFromSender[0], package);
  }
  else if (from == RECEIVER_ADDRESS){
    return packageInMemory(&packageMemoryFromReceiver[0], package);
  }
}
// Returns 1 if the package is in memory
int packageInMemory(int* packageMemory, int package){
  for (int i = 0; i<4; i++){
    if (packageMemory[i] == package){
      return 1;
    }
  }
  return 0;
}

// Pretends to be the original sender, and sends to the original destination
bool forwardMessage(uint8_t* buf, int bufLen, uint8_t from, uint8_t to){
  // The following is supposed to replace the rest. 
  /*
  return manager.sendtoWaitRepeater(buf, bufLen, to, from);
   */
  if(from == SENDER_ADDRESS){
    // The originator of the message is the sender
    return manager.sendtoWaitRepeater(buf, bufLen, RECEIVER_ADDRESS, SENDER_ADDRESS);
  }
  else if (from == RECEIVER_ADDRESS){
    return manager.sendtoWaitRepeater(buf, bufLen, SENDER_ADDRESS, RECEIVER_ADDRESS);
  }
}

// Returning smalles position of unused memory
// Returning 0 if memory is full
int findEmptyMemory(){    
  for (int i = 0; i < 4; i++){ // Assuming memory has size of 4
    if (memorySender[i][1] == 0){     
      return i;
    }
  }
  return 0; // No memory is empty, overwriting should start at pos 0
}
void writeToMemory(uint8_t* message, int messageLength){ 
  int _emptyMemory = findEmptyMemory();
  for(int i = 0; i < messageLength; i++){
    memorySender[_emptyMemory][i] = message[i];
  }
}



// sending all elements of memory, returns 1 for success
int sendFromMemory(uint8_t memory[][24], uint8_t author, uint8_t endDest){ 
  for (int i = 0; i <4; i++){ // Iterating over the 4 memory positions
    if (memory[i][1] != (char)0){
      // Don't try to send empty memory      
      if (manager.sendtoWaitRepeater(&memory[i][0], sizeof(memory[i]), endDest, author)){ // Trying to send from memory
        Serial.println(F("Send from memory success"));
        deleteFromMemory(i, author);          
      }
      else{
        // Not success  
        Serial.println(F("Send from memory not a success"));  
      }       
    }        
  }
}

void deleteFromMemory(int deletePosition, uint8_t author){
  if (author == SENDER_ADDRESS){
    memorySender[deletePosition][0] = (char)0;
    memorySender[deletePosition][1] = (char)0;    
  } 
}

//void printPackageMemory(){
//  Serial.println("Package memory from Sender: ");
//  for (int i = 0; i < 4; i++){
//    if (packageMemoryFromSender[i] != -1){
//      Serial.println(packageMemoryFromSender[i]);
//    }    
//  }
//  Serial.println("Package memory from Receiver: ");
//  for (int i = 0; i < 4; i++){
//    if (packageMemoryFromReceiver[i] != -1){
//      Serial.println(packageMemoryFromReceiver[i]);
//    }    
//  }
//}

void printReceived(uint8_t* message, uint8_t from){
  Serial.print(F("Received message from: "));
  Serial.print(from, HEX);
  Serial.print(F(" : "));
  Serial.print((int)message[0]);
  Serial.print((char*)&message[1]);
  Serial.print(F(" : "));  
}
void printMemory(){
  Serial.println(F("Memory:"));
  for (int i = 0; i < 4 ; i++){
    if (memorySender[i][1] != (char)0){
      Serial.print(F("For iterator value: "));
      Serial.print(i);
      Serial.print(F(" : "));
      Serial.print((int)memorySender[i][0]); // The number that is in front of the message
      // Serial.println((char*)&memorySender[i][1]); // The text in the message
    }    
  }
  Serial.println();
}

