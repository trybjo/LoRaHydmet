
void initializeLoRa()
{
  //Set pinMode
  pinMode(LoRa_RST, OUTPUT);
  pinMode(LoRa_INT, OUTPUT);

  // Manual reset of LoRa module
  digitalWrite(LoRa_RST, HIGH); 
  delay(100);
  digitalWrite(LoRa_RST, LOW);
  delay(10);
  digitalWrite(LoRa_RST, HIGH);
  delay(10);
  digitalWrite(LoRa_INT, HIGH);
  delay(100);
  digitalWrite(LoRa_INT, LOW);
  delay(10);
  digitalWrite(LoRa_INT, HIGH);
  delay(10);
  pinMode(LoRa_INT, OUTPUT);

  // Initialize both LoRa and Reliable Datagram
  if (!manager.init())
  Serial.println(F("Init failed"));
  delay(1000);

  // Initialize LoRa.
  //  lora.init();

  // Set frequency.
  lora.setFrequency(LoRa_FREQ);
  
  // Set transmitter power, value from 7-23 (23 = 20 dBm). 13 dBm is default. 
  // Can go above this using PA_BOOST, which RFM9x has.
  lora.setTxPower(23);

  // Set spreding factor, bandwidth and coding rate. 
  // See LoRa #define in top of code for description and usage.
//  LoRa.setSpreadingFactor(spreadingFactor); // This causes failure
  LoRa.setSignalBandwidth(signalBandwidth);
  LoRa.setCodingRate4(codingRateDenominator);
  delay(200);
}


// The function calls receive function with or without timeout
// Detects duplicates and update package memory
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
//      printReceived(&buf[0], from[0]);
    }
    else {
      //Serial.println(F("Received duplicate"));
      *duplicate = true;
    }
    return true;
  }
  else{
    *duplicate = false;
    return false;
  }
}

// This function aims to save SRAM by not using Serial.print(float)
// Replaces uint8PosToLongInt and uint8PosToFloat, and the Serial calls.

// Decoding uint8_t* to integer or decimal values and writing to serial.
// When 'decimals' == 0, the integer value will be printed.
void sPrintData(uint8_t* input, int usedSize, int startPos, int decimals){
  long int _temp = 0;
  for (int i = startPos; i< startPos + usedSize; i++){
    _temp += (long int)input[i] * toPowerOf(256, i-startPos); 
  }
  if (!decimals){
    Serial.print(_temp);
  }
  else{
    long int tenPower = toPowerOf(10,decimals);
    Serial.print(_temp/tenPower); // Integer value
    Serial.print(F("."));
    Serial.print(_temp - tenPower*(_temp/tenPower)); // Decimals
  }
}

void updatePackageNum(){
  if (packageNum >= 7){ packageNum = 0;} 
  else packageNum ++;
}

long int toPowerOf(int input, int power){
    long int temp = 1;
    for (int i = 0; i < power; i++){
      temp *= input;
    }
    return temp; 
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



void writeDataToSerial(uint8_t* message){
  //sPrintData(input, usedSize, startPos, decimals);
  
  // Signal strength dBm
  Serial.print((int)lora.lastRssi()); // THis one need to be chagned still
  Serial.print(F(","));
  // Packet number:
  sPrintData(message, 1, 0, 0);
  Serial.print(F(","));
  // Temperature:    
  sPrintData(message, 2, 1, 2);
  Serial.print(F(","));
  // Humidity:
  sPrintData(message, 2, 3, 1);
  Serial.print(F(","));
  // Pressure:
  sPrintData(message, 3, 5, 0);
  Serial.print(F(","));
  // Debth:
  sPrintData(message, 3, 8, 0);
  Serial.print(F(","));
  // Time: 
  sPrintData(message, 4, 11, 0);
  Serial.println();
}
