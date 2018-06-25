// Arduino9x_RX
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messaging client (receiver)
// with the RH_RF95 class. RH_RF95 class does not provide for addressing or
// reliability, so you should only use RH_RF95 if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example Arduino9x_TX

#include <SPI.h>
#include <RH_RF95.h>

#define RFM95_CS 10
#define RFM95_RST 3
#define RFM95_INT 2

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 868.0



// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

// Blinky on receipt
#define LED 13

void setup() 
{
  pinMode(LED, OUTPUT);     
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  while (!Serial);
  Serial.begin(9600);
  delay(100);

//  Serial.println("Arduino LoRa RX Test!");
  
  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init()) {
//    Serial.println("LoRa radio init failed");
    while (1);
  }
//  Serial.println("LoRa radio init OK!");

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
//  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);

  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(13, false);
}


void loop()
{
  if (rf95.available())
  {
    // Should be a message for us now   
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    
    if (rf95.recv(buf, &len))
    {
      digitalWrite(LED, HIGH);
//      RH_RF95::printBuffer("Received: ", buf, len);
      Serial.print("Temp: ");
      //uint8PosToFloat(uint8_t* input, int usedSize, int startPos, int decimals)
      Serial.print(uint8PosToFloat(buf, 2, 0, 2));
      
      Serial.print("*C Humidity: ");
      Serial.print(uint8PosToFloat(buf, 2, 2, 2));
      Serial.print("% Pressure: ");
      Serial.print(uint8PosToLongInt(buf, 3, 4));
      Serial.print("Pa Time: ");
      Serial.print(uint8PosToLongInt(buf, 3, 7));
      Serial.println("  DDHHMM");
      
      //Serial.println((char*)buf);
//       Serial.print("RSSI: ");
//      Serial.println(rf95.lastRssi(), DEC);
      
      // Send a reply
      uint8_t data[] = "And hello back to you";
      rf95.send(data, sizeof(data));
      rf95.waitPacketSent();
//      Serial.println("Sent a reply");
      digitalWrite(LED, LOW);
    }
    else
    {
//      Serial.println("Receive failed");
    }
  }
}

long int toPowerOf(int input, int power){
//  if (power == 0){
//    return 1;
//  }
//  else{
    long int temp = 1;
    for (int i = 0; i < power; i++){
      temp *= input;
    }
    return temp;
//  }  
}
float uint8PosToFloat(uint8_t* input, int usedSize, int startPos, int decimals){
  long int _temp = 0;
  for (int i = startPos; i< startPos + usedSize; i++){
    _temp += (long int)input[i] * toPowerOf(256, i-startPos); 
  }
  return (float)_temp/toPowerOf(10, decimals);  
}
long int uint8PosToLongInt(uint8_t* input, int usedSize, int startPos){
  long int _temp = 0;
  for (int i = startPos; i< startPos + usedSize; i++){
    _temp += (long int)input[i] * toPowerOf(256, i-startPos); 
  }
  return _temp;  
}






