


#include <EEPROM.h>
#include <Wire.h>


/*---------------------------------------------------------------------*
 * Write message to the memory according to the datasheet for repeater * 
 *---------------------------------------------------------------------*/
void writeMessageToMemory(uint8_t* message, int messageLength, uint8_t from);


/*--------------------------------------------------------------------------*
 * This function checks if the system starts for the very first time        * 
 * Indication of this is in memory position 90, it is 1 if the              *
 * system has been started earlier.                                         *
 * If first start, the system erase data from all relevant memory positions *
 *--------------------------------------------------------------------------*/
bool initializeMemory();

/*-----------------------------------------------------------------------* 
 * Sets memory position 11 of message relating author and packetNum to 0 * 
 * author is a value in range 1-7 representing sender address            *
 * packetNum is a value in range 0-7                                     *
 *-----------------------------------------------------------------------*/
void deleteFromMemory(uint8_t author, int packetNum);


/*--------------------------------------------------------------------------*
 * Stores the package number to memory corresponding to the sender          *
 * Updates the pointer to memory position containing latest package number  *
 *--------------------------------------------------------------------------*/
void updatePackageNumberMemory(int package, uint8_t from);
 
/*----------------------------------------------------------------*
 * Iterating trough package number memory for the given sender    *
 * Returns true if the value of packageNum is stored in memory    *
 * Returns false if packageNum is unlike all stored values        *
 *----------------------------------------------------------------*/
bool packageInMemory(int packageNum, uint8_t from);