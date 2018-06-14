// rf95_reliable_datagram_server.pde
// -*- mode: C++ -*-
// Example sketch showing how to create a simple addressed, reliable messaging server
// with the RHReliableDatagram class, using the RH_RF95 driver to control a RF95 radio.
// It is designed to work with the other example rf95_reliable_datagram_client
// Tested with Anarduino MiniWirelessLoRa, Rocket Scream Mini Ultra Pro with the RFM95W 

#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <SPI.h>


#define RFM95_CS 4
#define RFM95_RST 2
#define RFM95_INT 3

#define SENDER_ADDRESS 1
#define REPEATER_1_ADDRESS 2
#define RECEIVER_ADDRESS 3
#define RF95_FREQ 868.0

// Singleton instance of the radio driver
RH_RF95 driver;
//RH_RF95 driver(5, 2); // Rocket Scream Mini Ultra Pro with the RFM95W

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(driver, REPEATER_1_ADDRESS);


void setup() 
{
  Serial.begin(9600);
  while (!Serial) ; // Wait for serial port to be available
  if (!manager.init())
    Serial.println("init failed");
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 5 to 23 dBm:
//  driver.setTxPower(23, false);
  // You can optionally require this module to wait until Channel Activity
  // Detection shows no activity on the channel before transmitting by setting
  // the CAD timeout to non-zero:
//  driver.setCADTimeout(10000);
}

uint8_t data[] = "Ack";
// Dont put this on the stack:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

void loop()
{
  if (manager.available())
  {
    // Wait for a message addressed to us from someone
    uint8_t len = sizeof(buf);
    uint8_t from;
    if (manager.recvfromAck(buf, &len, &from))
    {
      Serial.print("got request from : 0x");
      Serial.print(from, HEX);
      Serial.print(": ");
      Serial.println((char*)buf);
      Serial.println(from);

      // Send a reply back to the originator client
      if (!manager.sendtoWait(data, sizeof(data), from)){
        Serial.println("sendtoWait failed");}
      else if (from == SENDER_ADDRESS){ // The message was from the sender, we forward to receiver
        Serial.println("Message from sender, trying to send to receiver");
        if (manager.sendtoWait(data, sizeof(data), RECEIVER_ADDRESS))
          // Now wait for a reply from the repeater
          uint8_t len = sizeof(buf);
          uint8_t from;   
          if (manager.recvfromAckTimeout(buf, &len, 2000, &from)){
            Serial.print("got reply from : 0x");
            Serial.print(from, HEX);
            Serial.print(": ");
            Serial.println((char*)buf);
          }
          else{
            Serial.println("No reply, is Receiver running?");
          }
        }
        else if (from == RECEIVER_ADDRESS){ // The message was from the receiver, we forward to sender
        if (manager.sendtoWait(data, sizeof(data), SENDER_ADDRESS))
          // Now wait for a reply from the repeater
          uint8_t len = sizeof(buf);
          uint8_t from;   
          if (manager.recvfromAckTimeout(buf, &len, 2000, &from)){
            Serial.print("got reply from : 0x");
            Serial.print(from, HEX);
            Serial.print(": ");
            Serial.println((char*)buf);
          }
          else{
            Serial.println("No reply, is Server running?");
          }
        }
     }
  }
}

