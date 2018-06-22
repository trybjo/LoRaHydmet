// Brukes for å sette båndbredde, spredningsfaktor og codnign rate
 #include <LoRa.h>

uint8_t data[1];

// LoRa 9x_TX
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messaging client (transmitter)
// with the RH_RF95 class. RH_RF95 class does not provide for addressing or
// reliability, so you should only use RH_RF95 if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example LoRa9x_RX

#include <SPI.h>
#include <RH_RF95.h>
 
#define RFM95_CS 10
#define RFM95_RST 3
#define RFM95_INT 2

int minutes = 30;
int hours = 12;
int date = 22;


 
// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);
int16_t packetnum = 0;  // packet counter, we increment per xmission

// Setter ulike parametere for frekvens + hastighet/avstand. Se i bunnen av koden for eksempler på innstillinger.
#define RF95_FREQ 868.0
#define spreadingFactor 7
#define signalBandwidth 123E3
#define codingRateDenominator 5


  // Library to use I2C communucation.
  #include <Wire.h>

  // Library that some of Adafruit's sensors uses.
  #include <Adafruit_Sensor.h>
  
// Temperature and air pressure

  // Library used for temperature and air pressure readings from the BMP280 chip.
  #include <Adafruit_BMP280.h>
  
  // Define, MOSI, MISO, SCK and chip celect pin.
  #define BMP_MOSI 5
  #define BMP_MISO 6
  #define BMP_SCK 7
  #define TempAndPressure_CS 8

  // Set the CS pin.
  Adafruit_BMP280 BMP(TempAndPressure_CS, BMP_MOSI, BMP_MISO, BMP_SCK);



// Humidity sensor

  // Library used by the AM2329 sensor.
  #include "Adafruit_AM2320.h"

  // Instance for the humidity sensor.
  Adafruit_AM2320 am2320 = Adafruit_AM2320();



  

void setup() 
{
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
 
  while (!Serial);  // Remove if not connected to a compoter!
  Serial.begin(9600);
  delay(100);
 
  Serial.println("Arduino LoRa TX Test!");
 
  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);
  initializeTempAndPressure();
  initializeHumidity();
 
  while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    while (1);
  }
  Serial.println("LoRa radio init OK!");
 
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);
  
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
 
  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 5 to 23 dBm:
   rf95.setTxPower(23);
   
LoRa.setSpreadingFactor(spreadingFactor);
LoRa.setSignalBandwidth(signalBandwidth);
LoRa.setCodingRate4(codingRateDenominator);

}
void loop()
{
  addToMinutes(600000);
  Serial.print("Date, Hour, Minute: ");
  Serial.print(date);
  Serial.print(hours);
  Serial.println(minutes);
  long int DDHHMM = minutes + 100* (long int)hours + 10000 *(long int)date;
  Serial.println(DDHHMM);
  Serial.println("Sending to rf95_server");
  // Send a message to rf95_server
  
  long int temp = (long int) (getTemp()*100); // Temp with two decimal precision
  long int humidity = (long int)(getHumidity()*100); // Humidity with two decimals
  long int pressure = (long int)getPressure(); // Pressure, no decimals

  // Total data, date(3 bytes), assuming temp(2 bytes), humidity(2 bytes), pressure(3 bytes)
  uint8_t totalData[10];
  // (inValue, requiredSize, startingPos, outValue)
  fillLongIntToPos(temp, 2, 0, totalData);
  fillLongIntToPos(humidity, 2, 2, totalData);
  fillLongIntToPos(pressure, 3, 4, totalData);  
  fillLongIntToPos(DDHHMM, 3, 7, totalData);
  
  Serial.print("Temperature: ");
  Serial.println((long int)totalData[0] + (long int)totalData[1]*256);
  Serial.print("Humidity: ");
  Serial.println((long int)totalData[2] + (long int)totalData[3] * 256 );
  Serial.print("Pressure: ");
  Serial.println((long int)totalData[4] + (long int)totalData[5] * 256 + (long int)totalData[6] * 256*256);
  Serial.print("Time: ");
  Serial.println((long int)totalData[7] + (long int)totalData[8] * 256 + (long int)totalData[9] * 256*256);
  
//  char radiopacket[20] = "Forbi trafoen! #      ";
//  itoa(packetnum++, radiopacket+13, 10);
  itoa(packetnum++, (char*)totalData+13, 10);
  Serial.print("Sending ");// Serial.println(radiopacket);
//  radiopacket[19] = 0;
  
  Serial.println("Sending..."); delay(10);
//  rf95.send((uint8_t *)radiopacket, 20);
  rf95.send(totalData, 10);
 
  Serial.println("Waiting for packet to complete..."); delay(10);
  rf95.waitPacketSent();
  // Now wait for a reply
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);
 
  Serial.println("Waiting for reply..."); delay(10);
  if (rf95.waitAvailableTimeout(1000))
  { 
    // Should be a reply message for us now   
    if (rf95.recv(buf, &len))
   {
      Serial.print("Got reply: ");
      Serial.println((char*)buf);
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);    
    }
    else
    {
      Serial.println("Receive failed");
    }
  }
  else
  {
    Serial.println("No reply, is there a listener around?");
  }
  delay(60000);
}

 
void fillLongIntToPos(long int inValue, int requiredSize, int startingPos, uint8_t* outValue){
  uint8_t * _tempValue = (uint8_t *)&inValue;
  for (int i = 0; i < requiredSize; i++){
    outValue[startingPos + i] = _tempValue[i];
  }
}


// General

    // A mapping function that can return floating (decimal) values.
    float floatMap(float x, float in_min, float in_max, float out_min, float out_max)
    {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }



// Temperature and air pressure

    // Initialize BMP. Sets all necessary pinModes.
    void initializeTempAndPressure()
    {
      BMP.begin();
    }

    // Returns temperature [*C].
    float getTemp()
    {
      //return 45.482;
      return BMP.readTemperature();
    }

    // Returns pressure [Pa].
    float getPressure()
    {
      Serial.print("Pressure directly: ");
      Serial.println(BMP.readPressure());
      return BMP.readPressure();
    }

// Humidity sensor

    // Initialize humidity sensor. Sets pins automatically.
    void initializeHumidity()
    {
      am2320.begin();
    }

    // Returns the humidity [%].
    float getHumidity()
    {
      //return 15.2;
      Serial.print("Humidity directly: ");
      Serial.println(am2320.readHumidity());
      return am2320.readHumidity();
    }


// Only works if adding < 60 minutes
void addToMinutes(long int ms){
  int addedMinutes = ms/60000;
  if (minutes + addedMinutes >= 60){
    // We start a new hour
    minutes = minutes - 60 + addedMinutes;
    if (hours+1 > 23){
      hours = 0;
      date ++;
    }
    else {
      hours ++;
    }
  }
  else{
    minutes += addedMinutes;
  }
}








/*
// Setup Spreading Factor (6 ~ 12)
  rf95.setSpreadingFactor(7);
  
  // Setup BandWidth, option: 7800,10400,15600,20800,31250,41700,62500,125000,250000,500000
  //Lower BandWidth for longer distance.
  rf95.setSignalBandwidth(125000);
  
  // Setup Coding Rate:5(4/5),6(4/6),7(4/7),8(4/8) 
  rf95.setCodingRate4(5);
  

  //Different Combination for distance and speed examples: 
  Example 1: Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on. Default medium range
    rf95.setSignalBandwidth(125000);
    rf95.setCodingRate4(5);
    rf95.setSpreadingFactor(7);
  Example 2: Bw = 500 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on. Fast+short range
    rf95.setSignalBandwidth(500000);
    rf95.setCodingRate4(5);
    rf95.setSpreadingFactor(7);
  Example 3: Bw = 31.25 kHz, Cr = 4/8, Sf = 512chips/symbol, CRC on. Slow+long range
    rf95.setSignalBandwidth(31250);
    rf95.setCodingRate4(8);
    rf95.setSpreadingFactor(9);
  Example 4: Bw = 125 kHz, Cr = 4/8, Sf = 4096chips/symbol, CRC on. Slow+long range
    rf95.setSignalBandwidth(125000);
    rf95.setCodingRate4(8);
    rf95.setSpreadingFactor(12); 
  */

