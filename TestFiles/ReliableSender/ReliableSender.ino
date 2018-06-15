// rf95_reliable_datagram_client.pde
// -*- mode: C++ -*-
// Example sketch showing how to create a simple addressed, reliable messaging client
// with the RHReliableDatagram class, using the RH_RF95 lora to control a RF95 radio.
// It is designed to work with the other example rf95_reliable_datagram_server
// Tested with Anarduino MiniWirelessLoRa, Rocket Scream Mini Ultra Pro with the RFM95W 



#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <SPI.h>

#define SENDER_ADDRESS 1
#define RECEIVER_ADDRESS 2
#define RF95_FREQ 868.0

RH_RF95 lora;

// Class to manage message delivery and receipt,using the lora declared above
RHReliableDatagram manager(lora, SENDER_ADDRESS);


void setup() 
{
  Serial.begin(9600);
  while (!Serial) ; // Wait for serial port to be available
  if (!manager.init())
    Serial.println("init failed");
}


// Dont put this on the stack:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

void loop()
{
  uint8_t data[] = "Hello from the sender!";
  Serial.println("Sending to the receiver");
  // Send a message to manager_server
  char* reply;
  reply = ReliableSend(data, sizeof(data), RECEIVER_ADDRESS, buf, sizeof(buf));

    
  delay(6000);
}

char* ReliableSend(uint8_t* data, uint8_t dataLen, uint8_t address, uint8_t* buf, uint8_t bufLen){
  if (manager.sendtoWait(data, dataLen, address)){
    Serial.print("Send successfull, got the Ack");
  }
  else
    Serial.println("sendtoWait failed");
    return (char*)"sendtoWait failed";
}

