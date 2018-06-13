#include "LowPower.h"
// This file requieres the LowPower support functions

// Code for temperature sensor
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

#define BMP_SCK 13
#define BMP_MISO 12
#define BMP_MOSI 11 
#define BMP_CS 10
Adafruit_BMP280 bme(BMP_CS, BMP_MOSI, BMP_MISO,  BMP_SCK);
////////////////////////////////////////

void setup() {
  // Code for temperature sensor
  Serial.begin(9600);
  
  if (!bme.begin()) {  
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    while (1);
  }
  /////////////////////////////////////
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(10);
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  ADCSRA |= (1 << 7); // Enabeling ADC
  delay(100);

  // Code for temperature setup
  Serial.print("Temperature = ");
  Serial.print(bme.readTemperature());
  Serial.println(" *C");
    
  Serial.print("Pressure = ");
  Serial.print(bme.readPressure());
  Serial.println(" Pa");

  Serial.println();
  delay(100); // Delay in order to print all to terminal
  //////////////////////////////////
}
