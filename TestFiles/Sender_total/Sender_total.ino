/*
 * Pinout from Arduino to peripherals:
 * 
 * 5V   -> 
 * 
 * D8   -> Temp&Press CS
 * 
 * D10  -> LoRa CS
 * D11  -> Temp&Press MOSI + LoRa MOSI
 * D12  -> Temp&Press MISO + LoRa MISO
 * D13  -> Temp&Press SCK + LoRa SCK
 * 
 * 
 * SDA  -> Humidity sensor pin 2 (from left, seen from side with holes)
 * SCL  -> Humidity sensor pin 4 (from left, seen from side with holes)
 */



// General

  // SPI library easy use of SPI. 
  // MISO, MOIS and SCK hare handled automatically by the SPI-library. Chip select (CS)/Slave select (SS) must be set manually.
  // CS/SS pin can be any digital pin. The slave communicates with master when its CS pin is low (usually...).
  #include <SPI.h>

  // Library to use I2C communucation.
  #include <Wire.h>

  // Library that some of Adafruit's sensors uses.
  #include <Adafruit_Sensor.h>



// LoRa

  // Libraries used for LoRa communication.
  #include <RH_RF95.h>
  #include <LoRa.h>

  // Define pins for chip select, reset and interrut.
  #define LoRa_CS 10
  #define LoRa_RST 3
  #define LoRa_INT 2

  // Singleton instance of the radio driver.
  RH_RF95 lora(LoRa_CS, LoRa_INT);

  // Set frequency. The LoRa chip used here can operate at 868 Hz.
  #define LoRa_FREQ 868.0
  
  // Set spreading factor, 6 ~ 12. 
  // LoRa is a Chirp Spread Spectrum (CSS) modulation. This setting determines the number of chirps per bit of data.
  // At the highest value (SF 12), the link has higher range and is more robust, but the data rate is lowered significantly. 
  // The lowest value (SF 6) is a special case designed for the highest possible data rate. In this mode however, the length of the packet has to be known in advance. Also, CRC check is disabled.
  #define spreadingFactor 12

  // Set bandwidth, option: 7800,10400,15600,20800,31250,41700,62500,125000,250000,500000 [Hz].
  // Lower BandWidth for longer distance. Semtech recommends to keep it above or equal to 62.5 kHz to be compatible with the clock speed.
  #define signalBandwidth 125E3

  // Set coding rate, 5 ~ 8 (values are actually 4/x).
  // LoRa modem always performs forward error correction. The amount of these error checks is expressed by the coding rate. 
  // The lowest value (CR 4/5) results in higher data rate and less robust link, the highest value (CR 4/8) provides more robust link at the expense of lowering data rate.
  #define codingRateDenominator 8



// Temperature and air pressure

  // Library used for temperature and air pressure readings from the BMP280 chip.
  #include <Adafruit_BMP280.h>
  
  // Define chip celect pin.
  #define TempAndPressure_CS 8

  // Set the CS pin.
  Adafruit_BMP280 BMP(TempAndPressure_CS);



// Humidity sensor

  // Library used by the AM2329 sensor.
  #include "Adafruit_AM2320.h"

  // Instance for the humidity sensor.
  Adafruit_AM2320 am2320 = Adafruit_AM2320();




void setup() {
// Setup code that will run once:

  // Start serial communication. A fast baud rate enable the to GPS data fast enough to the monitor.
  // Must/can be deleted to save power when the code runs on the Atmel alone.
  Serial.begin(115200); 

  // Initialize SPI.
  SPI.begin();

  // Initializes the different modules.
  startLoRa();
  startTempAndPressure();
  startHumidity();

}




void loop() {
  // Main code that will run repeatedly:



}









// All functions to be used in void loop()



  // Temperature and air pressure

    // Initialize BMP. Sets all necessary pinModes.
    void startTempAndPressure()
    {
      BMP.begin();
    }

    // Returns temperature [*C].
    float getTemp()
    {
      return BMP.readTemperature();
    }

    // Returns pressure [Pa].
    float getPressure()
    {
      return BMP.readPressure();
    }



  // LoRa

    // Initialize LoRa.
    void startLoRa()
    {
      //Set pinMode
      pinMode(LoRa_RST, OUTPUT);

      // Manual reset of LoRa module
      //digitalWrite(LoRa_RST, HIGH); delay(100);
      digitalWrite(LoRa_RST, LOW);
      delay(10);
      digitalWrite(LoRa_RST, HIGH);
      delay(10);

      // Initialize LoRa.
      lora.init();

      // Set frequenzy.
      lora.setFrequency(LoRa_FREQ);
      
      // Set transmitter power, value from 7-23 (23 = 20 dBm). 13 dBm is default. Can go above this using PA_BOOST, which RFM9x has.
      lora.setTxPower(23);

      // Set spreding factor, bandwidth and coding rate. See LoRa #define in top of code for description and usage.
      LoRa.setSpreadingFactor(spreadingFactor);
      LoRa.setSignalBandwidth(signalBandwidth);
      LoRa.setCodingRate4(codingRateDenominator);
    }



  // Humidity sensor

    // Initialize humidity sensor. Sets pins automatically.
    void startHumidity()
    {
      am2320.begin();
    }

    // Returns the humidity [%].
    float getHumidity()
    {
      return am2320.readHumidity();
    }
  


























