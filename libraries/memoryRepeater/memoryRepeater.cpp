
#include "memoryRepeater.h"
#include <Arduino.h>

#define SENDER1_ADDRESS 1
#define SENDER2_ADDRESS 2
#define SENDER3_ADDRESS 3
#define SENDER4_ADDRESS 4

#define RECEIVER_ADDRESS 5
#define REPEATER_ADDRESS 6


bool writeMessageToMemory(uint8_t* message, int messageLength, uint8_t from){
  int packageNum = message[0];
  if (from < RECEIVER_ADDRESS && messageLength < 22){
    for(int i = 0; i < messageLength; i++){
      EEPROM.update(100*packageNum + 22*(from-1) + i, message[i]);
    }
    EEPROM.update(100*packageNum + 22*(from-1) + 21, 1);
    return true;
  }

  else if (from == RECEIVER_ADDRESS && messageLength < 10){
    // We set the indicator for this message, and clear all others
    EEPROM.update(100*packageNum + 97, 1);
    for (int i = messageLength + 1; i < 8; i++){
      EEPROM.update(100*i + 97, 0);
    }
    for (int i = 0; i < messageLength; i++){
      EEPROM.update(100*i + 97, 0);
    }
    return true;
  }
  return false;
}

bool initializeMemory(){ 
  EEPROM.update(98,0); // Force memory whipe
  if(EEPROM.read(98) != 1){
    // Position 98 is 1 if the processor has been startet before
    // The processor is waking up from a reboot  
    // Deleting memory from all senders
    for (int i = 0; i < 8; i++){
      // i representing packet number
      for (int j = 1; j < 6; j++){
        // j representing sender addresses plus address of receiver
        deleteFromMemory(j, i);
      }
    }
    EEPROM.update(98,1); 
    // Indicating that the processor has been initiated
    return true;
  }
  return false;
}

void deleteFromMemory(uint8_t author, int packetNum){
  // Element 22 of each message is 0, indicating message is not in queue for sending
  // Because zero index, we add 21 to the first byte position
  // For the sender, byte 11 of message is what indicates message has been sent
  if (0 < author && author < 5 && 0 <= packetNum && packetNum  < 8){
    EEPROM.update(100*packetNum + 22*(author-1) + 21, 0);
  }
  else if (author == RECEIVER_ADDRESS && 0 <= packetNum && packetNum  < 8){
    EEPROM.update(100*packetNum + 22*(author-1) + 9, 0);
  }
}

// Calculates input ^ power
long int toPowerOf(int input, int power){
    long int temp = 1;
    for (int i = 0; i < power; i++){
      temp *= input;
    }
    return temp; 
}

// This function should be checked for bugs
bool messageIsNew(uint8_t* message, uint8_t from){
  
  int packetNum = message[0];
  
  if (from == RECEIVER_ADDRESS){
    // DO SOMETHING
    return (EEPROM.read(100*packetNum + 22*(from-1) + 9));
  }

  

  long int _timeMemory;
  long int _timeIncoming;
  for (int i = 11; i < 15; i++){
    // Iterating over the four bytes containing date and time
    _timeIncoming += (long int) message[i] * toPowerOf(256, i-11);
    _timeMemory += (long int) EEPROM.read(100*packetNum + 22*(from-1) + 11 + i) * toPowerOf(256, i-11);
  }
  // If the times are identical, we can easily tell that the message is not new.
  if (_timeMemory == _timeIncoming){
    return false;
  }

  // _timeMemory and _timeIncoming now both have the format: 
  // mmddyyhhMM 

  // We can have reached a new month
  int monthMemory = _timeMemory / 100000000;
  int monthIncoming = _timeIncoming / 100000000;
  if (monthMemory < 12 && monthMemory < monthIncoming){
    // We have just got to a new month
    return true;
  }
  else if (monthMemory == 12 && monthIncoming == 1){
    // We have just got to a new year
    return true;
  }  
  
  // We have not reached a new month
  // We can have reached a new day
  int dayMemory = (int)(_timeMemory / 1000000) % 100;
  int dayIncoming = (int)(_timeIncoming / 1000000) % 100;
  if (dayMemory < dayIncoming) return true;

  // We have not reached a new day
  // We can have reached a new hour
  int hourMemory = (_timeMemory % 1000) /100;
  int hourIncoming = (_timeMemory % 1000) /100;
  if (hourMemory < hourIncoming) return true;

  // We have not reached a new hour
  // We can have reached a new minute
  if (_timeMemory % 100 < _timeIncoming % 100) return true;
  
  // We don't have a new year, not a new month.
  // Not a new day, or a new hour.
  // Not a new minute.
  // The message is not new
  return false;
}

bool queuedForSending(int packetNum, uint8_t from){
  return EEPROM.read(100*packetNum + 22*(from-1) + 21);
}

bool packageInMemory(int packageNum, uint8_t from){
  if (from != RECEIVER_ADDRESS){
    if (EEPROM.read(100*packageNum + 22*(from-1) + 21)){
      return true;
    }
  }
  else if (from == RECEIVER_ADDRESS){
    if (EEPROM.read(100*packageNum + 22*(from-1) + 9)){
      return true;
    }
  }
  return false;
}