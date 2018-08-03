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
