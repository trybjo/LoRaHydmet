
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

// Trying to send all elements of memory from given author
// Pretending to be author when sending
void sendFromMemory(uint8_t author, uint8_t endDest){
  for (int i = 0; i < 8; i++){
    // Iterating over packet numbers
    if (queuedForSending(i, author)){
      // Only send messages queued for sending 
      uint8_t buf [messageLength];
      // Storing from memory to uint8_t*
      for (int j = 0; j < messageLength; j++){
        buf[j] = EEPROM.read(100*i + messageLength*(author-1) + j);
      }
      if (manager.sendtoWaitRepeater(&buf[0], messageLength, endDest, author)){ 
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
