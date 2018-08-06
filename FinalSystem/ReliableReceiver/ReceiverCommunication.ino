
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
    if (messageIsDuplicate(buf, *from)){
      // New message, we are interested
      *duplicate = true;
    }
    else {
      *duplicate = false;
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


void writeDataToSerial(uint8_t* message, int len, uint8_t from){
  //sPrintData(input, usedSize, startPos, decimals);
  
  // Signal strength dBm
  Serial.print((int)lora.lastRssi()); // This one need to be changed still
  Serial.print(F(","));
  // From: 
  Serial.print(from);
  Serial.print(F(","));
  // Time: 
  sPrintData(message, 4, 11, 0);
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
  // Latitude: 
  sPrintData(message, 3, 15, 5);
  Serial.print(F(","));
  // Longditude:
  sPrintData(message, 3, 18, 5);
  
  if(len > 21){
    // Repeater Latitude
    Serial.print(F(","));
    sPrintData(message, 3, 22, 5);
    Serial.print(F(","));
    // Repeater Longditude
    sPrintData(message, 3, 25, 5);
  }
  
  Serial.println();
}

















