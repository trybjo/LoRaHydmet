bool updateClock(int mode){
  if (mode == 1){
    Wire.begin();
    delay(50);
    if (! RTC.begin()) {
      while (1){
        Serial.println(F("Couldn't find rtc"));
      }
    } 
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
  else
  { 
    Wire.begin();
    Serial.println(F("GPS Start"));//Just show to the monitor that the sketch has started
    if (! RTC.begin()) {
      while (1){
        Serial.println(F("Couldn't find rtc"));
      }
    } 
    int iterator = 0;
    while (iterator < 4){
      while(Serial.available())//While there are characters to come from the GPS
      {
        gps.encode(Serial.read());//This feeds the serial NMEA data into the library one char at a time
      }
      if(gps.location.isUpdated() && gps.date.year() != 2000)
      {
        iterator ++;
      }
    }
    uint16_t yy = gps.date.year();
    uint8_t mm = gps.date.month();
    uint8_t dd = gps.date.day();
    uint8_t hh = gps.time.hour();
    uint8_t Min = gps.time.minute();
    uint8_t sec = gps.time.second();
    Serial.println(yy);
    Serial.println(mm);
    Serial.println(dd);
    Serial.println(hh);
    Serial.println(Min);
    Serial.println(sec);
    RTC.adjust(DateTime(yy, mm, dd, hh, Min, sec));
    Serial.println(TimeAlarm.getTimeStamp());
    delay(2000);
  }
}

void turnOnGPS(){
  DDRC |= 1 << GPS_POWER; // Output
  PORTC |= 1 << GPS_POWER; // HIGH GPS_POWER
  delay(40);
}
void turnOffGPS(){
  PORTC &= ~1 << GPS_POWER; // LOW, turn power off
}

bool initializePacketNum(){
  // Checks if it's the first time the module is used. If it isn't, it continues from the previous package number.
  if(EEPROM.read(50)!=0 && EEPROM.read(51)!=1 && EEPROM.read(52)!=2 && EEPROM.read(53)!=3 && EEPROM.read(54)!=4 && EEPROM.read(55)!=5 && EEPROM.read(56)!=6 && EEPROM.read(57)!=7)
  {
    for(int i=0; i<1025; i++)
    { 
      EEPROM.write(i, 0);
    }
    packageNum = 0;
  }
  else
  {
    for(int i=0; i<8; i++)
    {
      if(EEPROM.read(50+i) == i)
      {
        packageNum = i; 
      }
    }
  }  
}

void setSleepConfig(){
  DDRB = B00000000;
  DDRC = B00111100;   // SCL and SDA  and power for GPS and LoRa as output
  DDRD = B00110000;   // Power for clock and depth sensor set as output
  PORTB = ~DDRB;      // Pull all in-ports high
  PORTC = ~DDRC;      // Pull all in-ports high, turn off power for GPS and LoRa, set SCL and SDA low
  PORTD = ~DDRD;      // Pull all in-ports high, turn off power for clock and depth sensor
}

void setAwakePinConfig(){
  DDRB = B00111110;   // 
  DDRC = B00111100;
  DDRD = B00110000;  
  PORTB = ~DDRB;
  PORTC = ~DDRC;
  PORTD = ~DDRD;
}



// Initialize LoRa.
void initializeLoRa()
{  
  //DDRC |= 1 << LoRa_POWER;  // Output
  PORTC |= 1 << LoRa_POWER; // HIGH
  
  //Set pinMode
  pinMode(LoRa_RST, OUTPUT);
  pinMode(LoRa_INT, OUTPUT);

  // Manual reset of LoRa module
  digitalWrite(LoRa_RST, HIGH); 
  delay(100);
  digitalWrite(LoRa_RST, LOW);
  delay(10);
  digitalWrite(LoRa_RST, HIGH);
  delay(10);
  digitalWrite(LoRa_INT, HIGH);
  delay(100);
  digitalWrite(LoRa_INT, LOW);
  delay(10);
  digitalWrite(LoRa_INT, HIGH);
  delay(10);
  pinMode(LoRa_INT, OUTPUT);
  
  
  // Initialize both LoRa and Reliable Datagram
  if (!manager.init()){
    // init failed
    Serial.println(F("init failed"));
  }
  else{
    Serial.println(F("LoRa init okay"));
  }
  
  
  // Set frequency.
  lora.setFrequency(LoRa_FREQ);
  
  // Set transmitter power, value from 7-23 (23 = 20 dBm). 13 dBm is default. 
  // Can go above this using PA_BOOST, which RFM9x has.
  lora.setTxPower(23);
  
  // Set spreding factor, bandwidth and coding rate. 
  // See LoRa #define in top of code for description and usage.
  //  LoRa.setSpreadingFactor(spreadingFactor); // This causes failure
  LoRa.setSignalBandwidth(signalBandwidth);
  
  LoRa.setCodingRate4(codingRateDenominator);
}

void activateClock(){
  DDRD |= 1 << clockPowerPin; // Setting pin 4 as output
  PORTD |= 1 << clockPowerPin; // Setting pin 4 high
  Wire.begin(); // Setting up SCL and SDA;
  delay(40);
}

void initializeAlarm()
{
  //clear any pending alarms
  RTC.armAlarm(1, false);
  RTC.clearAlarm(1);
  RTC.alarmInterrupt(1, false);
  RTC.armAlarm(2, false);
  RTC.clearAlarm(2);
  RTC.alarmInterrupt(2, false);

  // Set interrupt mode
  RTC.writeSqwPinMode(DS3231_OFF);
}





void initializeWaterPressureSensor()
{
  mySDI12.begin();
  delay(20);
}
