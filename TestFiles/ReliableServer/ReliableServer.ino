// rf95_reliable_datagram_server.pde
// -*- mode: C++ -*-
// Example sketch showing how to create a simple addressed, reliable messaging server
// with the RHReliableDatagram class, using the RH_RF95 driver to control a RF95 radio.
// It is designed to work with the other example rf95_reliable_datagram_client
// Tested with Anarduino MiniWirelessLoRa, Rocket Scream Mini Ultra Pro with the RFM95W 

#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <SPI.h>

#define CLIENT_ADDRESS 1
#define SERVER_ADDRESS 2

// Singleton instance of the radio driver
RH_RF95 driver;
//RH_RF95 driver(5, 2); // Rocket Scream Mini Ultra Pro with the RFM95W

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(driver, SERVER_ADDRESS);

// Need this on Arduino Zero with SerialUSB port (eg RocketScream Mini Ultra Pro)
//#define Serial SerialUSB

void setup() 
{
  // Rocket Scream Mini Ultra Pro with the RFM95W only:
  // Ensure serial flash is not interfering with radio communication on SPI bus
//  pinMode(4, OUTPUT);
//  digitalWrite(4, HIGH);

  Serial.begin(9600);
  while (!Serial) ; // Wait for serial port to be available
  if (!manager.init())
    Serial.println("init failed");
}

uint8_t data[] = "And hello back to you";
// Dont put this on the stack:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

void loop()
{
  if (manager.available())
  {
    // Wait for a message addressed to us from the client
    uint8_t len = sizeof(buf);
    uint8_t from;
    char* receivedData;
    receivedData = reliableReceive(buf, len, from); // from is called by reference
    Serial.print("Message recieved: ");
    Serial.print(receivedData);
    Serial.print("got request from : 0x");
    Serial.print(from, HEX);

   sendReply(data, sizeof(data), from);
    }
  }

char* reliableReceive(uint8_t* buf, uint8_t bufLen, uint8_t &from){
  if (manager.recvfromAck(buf, &bufLen, &from)){
    return (char*)buf;
  }
  return 0;
}

int sendReply(uint8_t* data, uint8_t dataLen, uint8_t &from){
  Serial.println("Trying to send reply to: ");
  Serial.println(from);
  if (!manager.sendtoWait(data, dataLen, from)){
    Serial.println("sendtoWait failed");
    return 0;
  }
  else {
    Serial.println("Sending success");
    return 1;
  }
}

