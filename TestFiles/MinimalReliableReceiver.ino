
#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <SPI.h>

#define SENDER_ADDRESS 1
#define RECEIVER_ADDRESS 2

RH_RF95 driver;
RHReliableDatagram manager(driver, RECEIVER_ADDRESS); // Instanciate this object with address.
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];



void setup() {
  Serial.begin(9600);
  while (!Serial) ; // Wait for serial port to be available
  if (!manager.init())
  Serial.println("init failed");
}

void loop() {
  if (manager.available()){
    
    // Receiving messages
    uint8_t bufLen = sizeof(buf); // Needs to be here to convert to uint8_t
    uint8_t from;
    if (!manager.recvfromAck(buf, &bufLen, &from)){ //Function changes value of buf, bufLen and from
      Serial.println("Receive unsuccessful");
    }
    else{
      Serial.print("Received message from 0x");
      Serial.print(from, HEX);
      Serial.println(": ");
      Serial.println((char*)buf);    
    }
    int receivingBit = 1; // 1 when receiving
    int timeout = 700; // ms
    while (receivingBit){
      if (!manager.recvfromAckTimeout(buf, &bufLen, timeout, &from)){ // Listening for incoming messages 
       Serial.println("Receive failed, assuming no more messages for this time");
       receivingBit = 0;
      }
      else{
       Serial.print("Received message from 0x");
       Serial.print(from, HEX);
       Serial.println(": ");
       Serial.println((char*)buf);    
      }
    } 

    // After a successful receive(s), send a message: 
    uint8_t data[] = "And hello back to you";
    if (!manager.sendtoWait(data, sizeof(data), from)){
      Serial.println("Sending failed");
    }
    else Serial.println("Sending successful");
    }
}
