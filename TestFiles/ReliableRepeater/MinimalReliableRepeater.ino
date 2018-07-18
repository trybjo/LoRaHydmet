#include <RH_Repeater_ReliableDatagram.h>
#include <RH_RF95.h>
#include <SPI.h>
#include <EEPROM.h>


#define SENDER1_ADDRESS 1
#define SENDER2_ADDRESS 2
#define SENDER3_ADDRESS 3
#define SENDER4_ADDRESS 4

#define RECEIVER_ADDRESS 5
#define REPEATER_ADDRESS 6
#define RF95_FREQ 868.0
#define MESSAGE_LEN 15

RH_RF95 lora;     // Instanciate the LoRA driver

// Class to manage message delivery and receipt,using the lora declared above
RH_Repeater_ReliableDatagram manager(lora, REPEATER_ADDRESS);



// Don't put this on the stack:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

// Dynamic variables:
uint8_t packageNumberPointerReceiver = 0; // Points at next place to write packet number to memory
int memoryPointer1 = 0;
uint8_t packageNumberPointer1 = 0;
int memoryPointer2 = 0;
uint8_t packageNumberPointer2 = 0;
int memoryPointer3 = 0;
uint8_t packageNumberPointer3 = 0;
int memoryPointer4 = 0;
uint8_t packageNumberPointer4 = 0;


void setup() {
  Serial.begin(9600);
  while (!Serial) ; // Wait for serial port to be available
  if (!manager.init())
  Serial.println(F("init failed"));
  delay(1000); 
  initializeMemory();
  Serial.println(F("Setup repeater completed"));
  uint8_t testFrom = 1;
}


void loop() {
  if (manager.available()){
        
    uint8_t bufLen = sizeof(buf); // Needs to be here to convert to uint8_t
    uint8_t from;  // from becomes author of the message
    uint8_t to; // to becomes the intended receiver of the message
    bool duplicate;

    // receive(bool, message, bufLen, from, timeout);
    bool receiveSuccess = myReceive(duplicate, buf, &bufLen, &from, &to, 6000);
    if (receiveSuccess && !duplicate){
      if (!forwardMessage(buf, bufLen, from, to)){ 
        // The forwarding was not successful, need to write to memory
        if (from != RECEIVER_ADDRESS){
          // We only store messages from the sender             
          writeToMemory(buf, bufLen, from); 
          printMemory(from);
        }         
      }
      else Serial.println(F("Forward success"));
    }
//    sendFromMemory();
    Serial.print(F("Memory pointer: "));
    Serial.print(memoryPointer1);
    Serial.print(F(", Package number pointer: "));
    Serial.println(packageNumberPointer1);
    sendFromMemory();
    //Serial.println(F("That's it for this time"));
  }
}

void initializeMemory(){  
  if(EEPROM.read(42) == 1){
    // Position 42 is 1 if the processor has been startet before
    // The processor is waking up from a reboot
    loadDynamicVariables();
  }
  else{
    // Element 11 of each message is 0, indicating empty memory
    Serial.println(F("First time, initializing values to zero"));
    for (int i = 0; i<4; i++){
      EEPROM.update(100*SENDER1_ADDRESS + 15*i + 11, 0);
      EEPROM.update(100*SENDER2_ADDRESS + 15*i + 11, 0);
      EEPROM.update(100*SENDER3_ADDRESS + 15*i + 11, 0);
      EEPROM.update(100*SENDER4_ADDRESS + 15*i + 11, 0);
    }
    EEPROM.update(42,1);
  }
}

// Return 'value' if in range 0-3, else return 0
uint8_t valueLimit(uint8_t value){
  if (value <= 3 && value >=0){
    return value;
  }
  return 0;
}

void loadDynamicVariables(){
  // Pointers for the package number memory
  packageNumberPointerReceiver = valueLimit(EEPROM.read(100*RECEIVER_ADDRESS + 65));
  packageNumberPointer1 = valueLimit(EEPROM.read(100*SENDER1_ADDRESS + 65));
  packageNumberPointer2 = valueLimit(EEPROM.read(100*SENDER2_ADDRESS + 65));
  packageNumberPointer3 = valueLimit(EEPROM.read(100*SENDER3_ADDRESS + 65));
  packageNumberPointer4 = valueLimit(EEPROM.read(100*SENDER4_ADDRESS + 65));

  // Pointers for the memory of whole messages
  memoryPointer1 = valueLimit(EEPROM.read(100*SENDER1_ADDRESS + 60));
  memoryPointer2 = valueLimit(EEPROM.read(100*SENDER2_ADDRESS + 60));
  memoryPointer3 = valueLimit(EEPROM.read(100*SENDER3_ADDRESS + 60));
  memoryPointer4 = valueLimit(EEPROM.read(100*SENDER4_ADDRESS + 60));
}

void writeDynamicVariablesToMemory(){
  // Pointers for the package number memory
  EEPROM.update(100*RECEIVER_ADDRESS + 65, packageNumberPointerReceiver);
  EEPROM.update(100*SENDER1_ADDRESS + 65, packageNumberPointer1);
  EEPROM.update(100*SENDER2_ADDRESS + 65, packageNumberPointer2);
  EEPROM.update(100*SENDER3_ADDRESS + 65, packageNumberPointer3);
  EEPROM.update(100*SENDER4_ADDRESS + 65, packageNumberPointer4);

  // Pointers for the memory of whole messages
  EEPROM.update(100*SENDER1_ADDRESS + 60, memoryPointer1);
  EEPROM.update(100*SENDER2_ADDRESS + 60, memoryPointer2);
  EEPROM.update(100*SENDER3_ADDRESS + 60, memoryPointer3);
  EEPROM.update(100*SENDER4_ADDRESS + 60, memoryPointer4);
}

// Receives message and detects duplicates
bool myReceive(bool &duplicate, uint8_t* message, uint8_t* bufLen, uint8_t* from, uint8_t* to, int timeout){
  bool success;
  if (manager.recvfromAckTimeout(buf, bufLen, timeout, from, to)){
    // Receive successful
    success = true;
    if (!packageInMemory((int)buf[0], *from)){
      // New message, we are interested
      Serial.println(F("We are interested"));
      duplicate = false;
      updatePackageNumberMemory((int)buf[0], *from);
      printReceived(&buf[0], from[0]);
    }
    else {
      Serial.println(F("Received duplicate"));
      duplicate = true;
    }
  }
  else{
    duplicate = false;
    success = false;
  }
  return success;
}

// Stores the package number to memory corresponding to the sender
// Updates the pointer to memory position containing latest package number
void updatePackageNumberMemory(int package, uint8_t from){
  switch(from){
    case 1:
      updatePackageMemory(package, from, packageNumberPointer1);
      break;
    case 2:
      updatePackageMemory(package, from, packageNumberPointer2);
      break;
    case 3:
      updatePackageMemory(package, from, packageNumberPointer3);
      break;
    case 4:
      updatePackageMemory(package, from, packageNumberPointer4);
      break;
    case 5:
      updatePackageMemory(package, from, packageNumberPointerReceiver);
      break;
  }
}

void updatePackageMemory(int package, int from, uint8_t &memPointer){
  EEPROM.update(from*100+61+memPointer, package);
  updatePackagePointer(memPointer);
}

void updatePackagePointer(uint8_t &memPointer){
  if (memPointer < 3){
    memPointer ++;
  }
  else memPointer = 0;
}

// Returns true if the package is in memory
bool packageInMemory(int package, uint8_t from){
  Serial.print(F("Looking for: "));
  Serial.print(package);
  Serial.print(F(" among: "));
  for (int i = 0; i<4; i++){
    Serial.print(EEPROM.read(from*100+61+i));
    Serial.print(F(", "));
    if (EEPROM.read(from*100+61+i) == package){
      return true;
    }
  }
  Serial.println();
  return false;
}


// Pretends to be the original sender, and sends to the original destination
bool forwardMessage(uint8_t* buf, int bufLen, uint8_t from, uint8_t to){
  // The following is supposed to replace the rest. 
  return manager.sendtoWaitRepeater(buf, bufLen, to, from);
}

// Finds appropritate memory position and writes there
void writeToMemory(uint8_t* message, int messageLength, uint8_t from){ 
  int _emptyMemory = findEmptyMemory(from);
  for(int i = 0; i < messageLength; i++){
    EEPROM.update(100*from+15*_emptyMemory+i,message[i]);
  }
}

// returns lowest unused memory position for the spesific sender
// if no empty memory, returns value from 0-3, following the respective pointer.
int findEmptyMemory(uint8_t from){
  switch(from){
    case 1: 
      return findEmptyMemory(from, memoryPointer1);
    case 2: 
      return findEmptyMemory(from, memoryPointer2);
    case 3: 
      return findEmptyMemory(from, memoryPointer3);
    case 4: 
      return findEmptyMemory(from, memoryPointer4);
  }
}

int findEmptyMemory(uint8_t from, int& memPointer){
  for (int i = 0; i < 4; i++){
    if (EEPROM.read(100*from + 15*i + 11) == 0){ // Reading the month of the message, this is never 0
      // Memory position i is empty
      memPointer = i;
      return i;
    }
  }
  // Memory is full, we need to overwrite
  if (memPointer < 3){
    // We can increment legally
    memPointer ++;
    return memPointer;
  }
  memPointer = 0;
  return 0;
}

void sendFromMemory(){
  sendFromMemory(SENDER1_ADDRESS, RECEIVER_ADDRESS);
  sendFromMemory(SENDER2_ADDRESS, RECEIVER_ADDRESS);
  sendFromMemory(SENDER3_ADDRESS, RECEIVER_ADDRESS);
  sendFromMemory(SENDER4_ADDRESS, RECEIVER_ADDRESS);
}
// Trying to send all elements of memory from given author
void sendFromMemory(uint8_t author, uint8_t endDest){
  for (int i = 0; i <4; i++){
    if (EEPROM.read(100*author + 15*i + 11) != 0){
      // Don't try to send empty memory 
      uint8_t buf [MESSAGE_LEN];
      // Storing from memory to uint8_t*
      for (int j = 0; j < MESSAGE_LEN; j++){
        buf[j] = EEPROM.read(100*author + MESSAGE_LEN*i + j);
      }
      if (manager.sendtoWaitRepeater(&buf[0], MESSAGE_LEN, endDest, author)){ // Trying to send from memory
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
  EEPROM.update(100*author + 15*deletePosition + 11, 0);
}


void printPackageNumberMemory(int from){
  Serial.print(F("Package memory from "));
  Serial.print(from);
  Serial.println(F(": "));
  for (int i = 0; i < 4; i++){
    Serial.println(EEPROM.read(from*100+61+i)); 
  }
}

void printReceived(uint8_t* message, uint8_t from){
  Serial.print(F("Received message from: "));
  Serial.print(from, HEX);
  Serial.print(F(" : "));
  Serial.println((int)message[0]);  
}
void printMemory(int from){
  Serial.print(F("Memory from "));
  Serial.print(from);
  Serial.println(F(":"));
  
  for (int i = 0; i < 4 ; i++){
    if (EEPROM.read(100*from + 15*i + 11 != 0)){
      Serial.print(F("For iterator value: "));
      Serial.print(i);
      Serial.print(F(" : "));
      Serial.println(EEPROM.read(100*from + 15*i)); // The number that is in front of the message
    }    
  }
  Serial.println();
}

