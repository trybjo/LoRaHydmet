
// Receives message and detects duplicates
bool myReceive(bool &duplicate, uint8_t* message, uint8_t* bufLen, uint8_t* from, uint8_t* to, int timeout){
  bool success;
  if (manager.recvfromAckTimeout(buf, bufLen, timeout, from, to)){
    // Receive successful
    success = true;
    if (messageIsNew(buf, from)){
      // New message, we are interested
      duplicate = false;
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



// Pretends to be the original sender, and sends to the original destination
bool forwardMessage(uint8_t* buf, int bufLen, uint8_t from, uint8_t to){
  // The following is supposed to replace the rest. 
  return manager.sendtoWaitRepeater(buf, bufLen, to, from);
}

void sendFromMemory(){
  for (int i = 1; i < 5; i++){
    sendFromMemory(i, RECEIVER_ADDRESS);
  }
}
void getPosition(){
  int iterator = 0;
  
  while (iterator < 4){
    while(Serial.available())//While there are characters to come from the GPS
    {
      gps.encode(Serial.read());//This feeds the serial NMEA data into the library one char at a time
    }
    if(gps.location.isUpdated())//This will pretty much be fired all the time anyway but will at least reduce it to only after a package of NMEA data comes in
    {
      iterator ++;
    }
  }
  long int Lat = gps.location.lat() * 100000;
  long int Lng = gps.location.lng() * 100000;
  Serial.println(Lat);
  Serial.println(Lng);
  uint8_t positionData[6];
  fillLongIntToPos(Lat, 3, 0, positionData);
  fillLongIntToPos(Lng, 3, 3, positionData);
  for (int i = 0; i < 6; i++){
    // Writing the new position data to memory 802-807
    EEPROM.update(802 + i, positionData[i]);
  }
}


// Trying to send all elements of memory from given author
// Pretending to be author when sending
void sendFromMemory(uint8_t author, uint8_t endDest){
  for (int i = 0; i < 8; i++){
    // Iterating over packet numbers
    if (queuedForSending(i, author)){
      // Only send messages queued for sending 
      uint8_t message [messageLength + 6];
      // Storing from memory to uint8_t*
      for (int j = 0; j < messageLength; j++){
        message[j] = EEPROM.read(100*i + messageLength*(author-1) + j);
      }
      for (int j = 0; j < 6; j++){
        message[j + messageLength] = EEPROM.read(802 + j);
      }
      if (manager.sendtoWaitRepeater(message, messageLength + 6, endDest, author)){ 
        // Trying to send from memory
        
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

void fillLongIntToPos(long int inValue, int requiredSize, int startingPos, uint8_t* outValue){
  uint8_t * _tempValue = (uint8_t *)&inValue;
  for (int i = 0; i < requiredSize; i++){
    outValue[startingPos + i] = _tempValue[i];
  }
}

// Checks if this is the first message from a given sender. 
// If first message, checks if the message arrived too late or too early
// Sends request to change clock if the message arrived at wrong time
void sendTimingError(byte &firstReceive, uint8_t from){
  if (!((firstReceive >> from) & 1)){
    // If this is the first message we get from the given sender
    firstReceive |= (1<<from); // Indicate that we now have got a message from the sender
    int timeDiff = TimeAlarm.timeDifference() + 30 + 2*(from-2);
    // The repeater starts 30 seconds early, the senders are off by 2*(from-2) seconds 
    if (abs(timeDiff) > 15){
      // The time difference from now and when the sender should send message
      // Is larger than 15 seconds. 
      // We should send a message to that sender, and ask to change clock time
      Serial.print(F("We got a message too late: "));
      Serial.println(timeDiff);
      uint8_t message[2];
      if (timeDiff < 0){
        message[0] = 0;
      }
      else{
        message[0] = 1;
      }
      message[1] = abs(timeDiff);
      manager.sendtoWaitRepeater(message, sizeof(message), from, (uint8_t)REPEATER_ADDRESS);
    }        
  }
}



















