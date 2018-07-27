


#include <EEPROM.h>
#include <Wire.h>


/*----------------------------------------------------------------------------*
 * Write message to the memory according to the datasheet for repeater        * 
 * messageLength for senders is  maximum 22                                   *
 * from, if RECEIVER_ADDRESS, the indicator byte is set, and indicator bytes  *
 * for the other packets are cleared. Does not write message to memory.       *
 * returns true if messageLenth <= 22, and message is written to memory,      *
 * returns false if messageLength > 22, and message is not written to memory  *
 *----------------------------------------------------------------------------*/
bool writeMessageToMemory(uint8_t* message, int messageLength, uint8_t from);


/*--------------------------------------------------------------------------*
 * This function checks if the system starts for the very first time        * 
 * Indication of this is in memory position 98, it is 1 if the              *
 * system has been started earlier.                                         *
 * If first start, the system erase data from all relevant memory positions *
 *--------------------------------------------------------------------------*/
bool initializeMemory();

/*---------------------------------------------------------*
 * If from is RECEIVER_ADDRESS, returns the indicator byte *
 * Else, the month, date, hour and minute of 'message'     *
 * is compared with memory. If 'message' is newer in time: *
 * returns true.                                           *
 * Returns false if message is not new                     *
 *---------------------------------------------------------*/ 
bool messageIsNew(uint8_t* message, uint8_t from);

/*------------------------------------------------------------------*
 * Returns indicator byte for the given packet for the given sender *
 *------------------------------------------------------------------*/
bool queuedForSending(int packetNum, uint8_t from);

/*-----------------------------------------------------------------------* 
 * Sets memory position 11 of message relating author and packetNum to 0 * 
 * author is a value in range 1-7 representing sender address            *
 * packetNum is a value in range 0-7                                     *
 *-----------------------------------------------------------------------*/
void deleteFromMemory(uint8_t author, int packetNum);
 
/*----------------------------------------------------------------*
 * Iterating trough package number memory for the given sender    *
 * Returns true if the value of packageNum is stored in memory    *
 * Returns false if packageNum is unlike all stored values        *
 *----------------------------------------------------------------*/
bool packageInMemory(int packageNum, uint8_t from);