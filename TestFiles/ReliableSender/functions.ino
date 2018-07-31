//////////////// Reading measurements ////////////////
//////////////////////////////////////////////////////

int getDepth()
{
  String startMeasurements = "?M!";
  String getData = "?D0!";
  String SDI_Response = "";
  String SDI_Response_Stripped = "";
  digitalWrite(depthMOSFETS, HIGH);
  delay(5000);
  mySDI12.sendCommand(startMeasurements);
  delay(3000); // Allow three seconds for measuring
  mySDI12.sendCommand(getData);
  delay(300);   // wait a while for a response
  
  while (mySDI12.available()) {  // build string from response
      char c = mySDI12.read();
      
      if( (c=='0') || (c=='1') || (c=='2') || (c=='3') || (c=='4') || (c=='5') || (c=='6') || (c=='7') || (c=='8') || (c =='9')){
        SDI_Response += c;
        delay(5);
      }
  }
  SDI_Response_Stripped += SDI_Response[7];
  SDI_Response_Stripped += SDI_Response[8];
  SDI_Response_Stripped += SDI_Response[9];
  SDI_Response_Stripped += SDI_Response[10];
  int depth = SDI_Response_Stripped.toInt();
  delay(5);
  SDI_Response = "";
  SDI_Response_Stripped = "";
  mySDI12.clearBuffer();
  digitalWrite(depthMOSFETS, LOW);
  delay(5);
  Serial.print(F("Depth = "));
  Serial.println(depth);
  
  return depth;
}

// Returns the humidity [%].
long int getHumidity()
{

  return (long int) (am2320.readHumidity() * 10 ) ;
}

// Returns temperature [*C] with two decimals precision.
long int getTemperature()
{
  return (long int) (BMP.readTemperature() * 100);
}

// Returns pressure [Pa].
long int getPressure()
{
  return (long int) BMP.readPressure();
}

// Not tested
void getPosition(){
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
  long int Lat = gps.location.lat() * 100000;
  long int Lng = gps.location.lng() * 100000;
  uint8_t positionData[6];
  fillLongIntToPos(Lat, 3, 0, positionData);
  fillLongIntToPos(Lng, 3, 3, positionData);
  for(int i=0; i<8; i++)
  {
    for (int j = 0; j < 6; j++)
    {
      // Writing the new position data to memory of all packets 
      EEPROM.update(i*100 + 15 + j, positionData[j]);
    }
  }
}

// Not tested
void fillPositionData(uint8_t outValue[messageLength]){
  for (int i = 15; i < 21; i++){
    outValue[i] = EEPROM.read(i);
  }
}

//------------------------------------------------------------------------------------------------------------------------


/////////////////// Other functions ///////////////////
//////////////////////////////////////////////////////

void enableSdaScl(){
  
}

void disableSdaScl(){
  DDRC |= B00110000;
  PORTC &= B11001111;  
}

// A mapping function that can return floating (decimal) values.
float floatMap(float x, float in_min, float in_max, float out_min, float out_max)
{
return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// counting package numbers 0-7 to try to use 3 bit at some time.
void updatePackageNum(){
  if (packageNum >= 7)
  { 
    packageNum = 0;
    EEPROM.write(50,0);
    EEPROM.write(57, 77);
  } 
  else
  {
    packageNum ++;
    EEPROM.write(50+packageNum, packageNum);
    EEPROM.write(50+packageNum-1, 77);
  }
}

// Iterating over memory, trying to send all of them
void sendFromEEPROM(){ 
  // Iterating over the 7 packet numbers
  for(int i=0; i<8; i++)
  { 
    // Only send data that exists (the month is never zero)   
    if(EEPROM.read(i*100+11) != 0)
    {  
      // Trying to send from memory
      uint8_t tempStorage[messageLength];
      for(int j=0; j<messageLength; j++)
      {
        tempStorage[j] = EEPROM.read(i*100+j);
        //Serial.println(tempStorage[j]);
      }
      if(!manager.sendtoWait(&tempStorage[0], messageLength, RECEIVER_ADDRESS))
      { 
        Serial.print(F("Send from memory unsuccessful for packet number: "));
        Serial.println(i);
      }
      else
      {
        Serial.println(F("Send from memory success"));
        EEPROM.update(i*100+11,0);       
      }   
    }      
  }
}

void printAllEEPROM()
{
  for(int i=0; i<8; i++)
  {
    for(int j=0; j<15; j++)
    {
      Serial.print(F("Adress "));
      Serial.print(i*100+j);
      Serial.print(" = ");
      Serial.println(EEPROM.read(i*100+j));
    }
  }
}

void printAlarm(){
  Serial.print(F("Alarm: "));
  int hh = EEPROM.read(846);
  int mm = EEPROM.read(847);
  int ss = EEPROM.read(848); 
  Serial.print(hh);
  Serial.print(mm);
  Serial.println(ss);
  delay(100);
}

void printReceived(uint8_t* message, uint8_t from){
  Serial.print(F("Received a message: "));
  Serial.print((int)message[0]);
  Serial.println((char*)&message[1]);
  Serial.print(F("Sender of the message was : 0x"));
  Serial.println(from, HEX);
}


void fillLongIntToPos(long int inValue, int requiredSize, int startingPos, uint8_t* outValue){
  uint8_t * _tempValue = (uint8_t *)&inValue;
  for (int i = 0; i < requiredSize; i++){
    outValue[startingPos + i] = _tempValue[i];
  }
}

void storeInEEPROM(uint8_t data[], int dataLen)
{
 for(int i=0; i<dataLen; i++)
 {
  EEPROM.update(data[0]*100+i, data[i]);
 }
}

void goToSafeSleep(){
  bool fakeWakeup = true;
  
  while (fakeWakeup){
    disableSdaScl();
    delay(40);
    goToSleep();
    //enableSdaScl();
    
    DateTime timeNow = RTC.now();
    bool correctHour = timeNow.hour() == EEPROM.read(846);
    bool correctMinute = timeNow.minute() == EEPROM.read(847);
    bool correctSecond = timeNow.second() == EEPROM.read(848);    
    if (correctHour && correctMinute && correctSecond){
      fakeWakeup = false;
    }
    else{
      Serial.println(F("We woke up, and we where not supposed to!"));
      Serial.print(F("The clock is: "));
      Serial.println(TimeAlarm.getTimeStamp());
      Serial.println(TimeAlarm.getTime());
      delay(100);
    }
  }
}

void goToSleep()
{
  attachInterrupt(digitalPinToInterrupt(clockInterruptPin), wakeUp, LOW);
  // Disable ADC
  ADCSRA &= ~(1<<7);
  
  // Go to deep sleep
  SMCR |= (1<<2); // Power down mode
  SMCR |= 1; // Enable sleep
  
  // BOD disable
  MCUCR |= (3<<5); // Set both BODS and BODSE at the same time
  MCUCR = (MCUCR & ~(1<<5)) | (1<<6); // Then set the BODS bit and clear the BODSE bit at the same time
  __asm__ __volatile__("sleep"); // In line assembler sleep execute instruction
  detachInterrupt(digitalPinToInterrupt(clockInterruptPin));
  /*
  // Allow wake up pin to trigger interrupt on low.
  attachInterrupt(0, wakeUp, FALLING);

  // Enter power down state with ADC and BOD module disabled.
  // Wake up when wake up pin is low.
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); 
  detachInterrupt(0); 
  */
}

void wakeUp()
{  
  // Just a handler for the pin interrupt.
}

void stopAlarm(){
  RTC.armAlarm(1, false);
  RTC.clearAlarm(1);
  RTC.alarmInterrupt(1, false);
}



long int getTime(){
  DateTime timeNow = RTC.now(); 
  // year = 2018, year%100 = 18
  long int timeInt = (long)timeNow.month() * 100000000 + (long)timeNow.day()*1000000 +((long)timeNow.year()%100)*10000;
  timeInt += (long)timeNow.hour()*100 + (long) timeNow.minute();
  return timeInt; 
}

// After performing a reboot/reset, watchdog timer must be turned off to not accidentally reboot again.
// Put this function at the top of the code in  setup().
void turnOffWatchdogTimer()
{
   MCUSR = 0;
   wdt_disable();

   return;
}

// Uses watchdog timer to reset the MCU
void rebootMCU()
{      
  do                          
  {                           
     wdt_enable(WDTO_30MS);  
     for(;;)                 
     {                       
     }                       
  } while(0);
}
