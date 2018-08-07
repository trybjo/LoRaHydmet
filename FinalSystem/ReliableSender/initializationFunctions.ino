
bool updateClock(int mode){
  if (mode == 1){
    if (! RTC.begin()) {
      while (1){
        Serial.println(F("Couldn't find rtc"));
      }
    } 
    RTC.adjust(DateTime(__DATE__, __TIME__));
    return true;
  }
  else
  {
    RTC.begin();
    // The following code uses sleep to save power when waiting for GPS signal
    /*
    bool connection = false; 
    int iterator = 0;
    for (int i = 0; i < 30; i++){
      // Searching for about 30 minutes, this should be more than enough
      powerNap(8); // Sleeping for 8x8 seconds ~ 1 min
      Serial.println(F("Woke"));
      delay(100);
      long int j = 0;
      bool notGivenUp = true;
      int numberOfUpdates = 0;
      //bool noData = true;
      while(!connection && notGivenUp) // Was noData
      {
        while(Serial.available())//While there are characters to come from the GPS
        {
             gps.encode(Serial.read());//This feeds the serial NMEA data into the library one char at a time
        }
        if(gps.location.isUpdated())//This will pretty much be fired all the time anyway but will at least reduce it to only after a package of NMEA data comes in
        {  
          Serial.println(F("Got data"));
          //noData = false;
          numberOfUpdates ++;
          if (numberOfUpdates > 4){
            connection = true;
            Serial.println(gps.location.lat(), 6);
          }
        }
        if(j>1000000)
        {
          Serial.println(F("Gives up this round"));
          notGivenUp = false;
        }
        j++;
      }
      if(connection == true){
        Serial.print(F("J = "));
        Serial.println(j);
        break;
      }
    }
    if (!connection){
      return false;
    }
    */
    
    int numberOfUpdates = 0;
    while (numberOfUpdates < 4){
      while(Serial.available())//While there are characters to come from the GPS
      {
           gps.encode(Serial.read());//This feeds the serial NMEA data into the library one char at a time
      }
      if(gps.location.isUpdated())//This will pretty much be fired all the time anyway but will at least reduce it to only after a package of NMEA data comes in
      {  
        numberOfUpdates ++;
      }
    }    
    uint16_t yy = gps.date.year();
    uint8_t mm = gps.date.month();
    uint8_t dd = gps.date.day();
    uint8_t hh = gps.time.hour();
    uint8_t Min = gps.time.minute();
    uint8_t sec = gps.time.second();
    
    RTC.adjust(DateTime(yy, mm, dd, hh, Min, sec));
    
    Serial.println(TimeAlarm.getTimeStamp());
    return true;
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

void activateHumiditySensor()
{
  am2320.begin();
  delay(5);
  // DDRD |= 1 << humidityPower; // Output
  PORTD |= 1 << humidityPower; // HIGH GPS_POWER
  delay(800); // Needs 800ms of power before getting readings. Will otherwise return 0.
}


void deactivateHumiditySensor()
{
  PORTD &= ~1 << humidityPower; // LOW, turn power off
}






