
/*
 * D4   -> Clock power
 * D5   -> Depth Multiplexer and MOSFET pin
 * 
 * 
 * A2   -> GPS MOSFET
 */
#define clockInterruptPin 3
#define SENDER_ADDRESS 2 // Range 1 to 4
#define depthDataPin A0
SDI12 mySDI12(depthDataPin); // Define the SDI-12 bus
#define messageLength 21
