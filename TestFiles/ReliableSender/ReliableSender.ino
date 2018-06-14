// rf95_reliable_datagram_client.pde
// -*- mode: C++ -*-
// Example sketch showing how to create a simple addressed, reliable messaging client
// with the RHReliableDatagram class, using the RH_RF95 driver to control a RF95 radio.
// It is designed to work with the other example rf95_reliable_datagram_server
// Tested with Anarduino MiniWirelessLoRa, Rocket Scream Mini Ultra Pro with the RFM95W 



#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <SPI.h>

#define SENDER_ADDRESS 1
#define RECEIVER_ADDRESS 2
#define RF95_FREQ 868.0

// Singleton instance of the radio driver
RH_RF95 driver;
//RH_RF95 driver(5, 2); // Rocket Scream Mini Ultra Pro with the RFM95W

// Class to manage message delivery and receipt,using the driver declared above
RHReliableDatagram manager(driver, SENDER_ADDRESS);


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
  if (manager.sendtoWait(data, dataLen, address))
  {
    // Now wait for a reply from the repeater
    uint8_t len = sizeof(buf);
    uint8_t from;   
    if (manager.recvfromAckTimeout(buf, &bufLen, 2000, &from))
    {
      
      Serial.print("got reply from : 0x");
      Serial.print(from, HEX);
      Serial.print(": ");
      Serial.println((char*)buf);
      return (char*)buf;
    }
    else
    {
      Serial.println("No reply, is Repeater1 running?");
      return (char*)"No reply, is Repeater1 running?";
    }
  }
  else
    Serial.println("sendtoWait failed");
    return (char*)"sendtoWait failed";
}

