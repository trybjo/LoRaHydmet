
// Converts long int to uint8_t*
// 'outValue' is an array that gets filled with the data in 'inValue'
// 'startingPos' is the first byte in outValue to get filled
// 'requiredSize' is in bytes, and needs to be calculated. If this value is set too low, 'outValue' is filled
// with wrong data
void fillLongIntToPos(long int inValue, int requiredSize, int startingPos, uint8_t* outValue){
  uint8_t * _tempValue = (uint8_t *)&inValue;
  for (int i = 0; i < requiredSize; i++){
    outValue[startingPos + i] = _tempValue[i];
  }
}

// Adds the position data stored in memory, to 'outValue'
// by calling this function, the argument array will get filled
void fillPositionData(uint8_t outValue[messageLength]){
  for (int i = 5; i < 11; i++){
    outValue[i] = EEPROM.read(i);
  }
}

void sendFromEEPROM(){ 
  // Iterating over the 7 packet numbers
  for(int i=0; i<8; i++)
  { 
    // Only send data that exists (the month is never zero)   
    if(EEPROM.read(i*100+11) != 0)
    {  
      // Trying to send from memory
      uint8_t tempStorage[messageLength];
      for(int j=0; j<messageLength; j++)
      {
        tempStorage[j] = EEPROM.read(i*100+j);
        //Serial.println(tempStorage[j]);
      }
      if(!manager.sendtoWait(&tempStorage[0], messageLength, RECEIVER_ADDRESS))
      { 
        Serial.print(F("Send from memory unsuccessful for packet number: "));
        Serial.println(i);
      }
      else
      {
        Serial.println(F("Send from memory success"));
        EEPROM.update(i*100+11,0);       
      }   
    }      
  }
}

// This function is created to make it possible to read messages sent from the repeater, 
// asking the sender to change the alarm time, to get in sync. 
// Because the function recvfromAckTimeout is depending on there being a message on the buffer, 
// this needs to be called frequently to make sure a message is read. 
// By calling receiveTimoutMessage(500), the function will listen for messages ten times, up to a total of 500 ms
// The if statements are created to match the spesific message from repeater, be aware of this when creating new 
// messages sent from repeater to sender
//
// There are currently bugs appearing when calling this function.
int receiveTimeoutMessage(int listenTime){
  uint8_t buf[3];                                                 //
  uint8_t bufLen = sizeof(buf);                                   // Needs to be here to convert to uint8_t
  uint8_t from;                                                   // from becomes author of the message
  for ( int j = 0; j < 10; j ++){
    if (manager.recvfromAckTimeout(buf, &bufLen, 200, &from)){    //
      if (from == REPEATER_ADDRESS && bufLen == 2){               // Special message from repeater 
        Serial.print(F("We got time request: "));                 //
        int seconds = buf[1];                                     // Number of seconds to change the clock
        if (!buf[0]){                                             //
          // Position 0 is 0 if the value of seconds is negative  //
          seconds = seconds * -1;                                 //
        }
        Serial.println(seconds);
        return seconds;
      }
    }
    delay(listenTime / 10);
  }
  return 0;
}

