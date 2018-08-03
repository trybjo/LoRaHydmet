// Initialize LoRa.
void initializeLoRa()
{  
  DDRC |= 1 << LoRa_POWER;  // Output
  PORTC |= 1 << LoRa_POWER; // HIGH, creates trouble  
  
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

void initializeAlarm()
{
  pinMode(wakeUpPin, INPUT);

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

void activateClock(){
  DDRD |= 1 << clockPowerPin; // Setting pin 4 as output
  PORTD |= 1 << clockPowerPin; // Setting pin 4 high
  Wire.begin(); // Setting up SCL and SDA;
  delay(40);
}

bool updateClock(int mode){
  if (mode == 1){
    if (! RTC.begin()) {
      while (1){
        Serial.println(F("Couldn't find rtc"));
      }
    } 
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
  else
  {
    DDRC |= 1 << GPS_POWER; // Output
    PORTC |= 1 << GPS_POWER; // HIGH GPS_POWER
    
    Serial.println("GPS Start");//Just show to the monitor that the sketch has started
    Wire.begin();
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
      if(gps.location.isUpdated())//This will pretty much be fired all the time anyway but will at least reduce it to only after a package of NMEA data comes in
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
    RTC.adjust(DateTime(yy, mm, dd, hh, Min, sec));
    
    Serial.println(TimeAlarm.getTimeStamp());
    PORTC &= ~1 << GPS_POWER; // LOW, turn power off
  }
}
