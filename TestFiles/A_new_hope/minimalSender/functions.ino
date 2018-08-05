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
  Serial.println(Lat);
  Serial.println(Lng);
  uint8_t positionData[6];
  fillLongIntToPos(Lat, 3, 0, positionData);
  fillLongIntToPos(Lng, 3, 3, positionData);
    for (int j = 0; j < 6; j++)
    {
      // Writing the new position data to memory of all packets 
      EEPROM.update(100 + 15 + j, positionData[j]);
    }
}





void fillLongIntToPos(long int inValue, int requiredSize, int startingPos, uint8_t* outValue){
  uint8_t * _tempValue = (uint8_t *)&inValue;
  for (int i = 0; i < requiredSize; i++){
    outValue[startingPos + i] = _tempValue[i];
  }
}

// Does the same as fillLongIntToPos, but for the GPS coordinates.
void fillPositionData(uint8_t outValue[messageLength]){
  for (int i = 15; i < 21; i++){
    outValue[i] = EEPROM.read(i);
  }
}





void goToSleep()
{
  attachInterrupt(digitalPinToInterrupt(clockInterruptPin), wakeUpRoutine, LOW);
  // Set pinMode:
  DDRB &= B11000000; // Not touching crystal
  DDRC &= B10000000; // Not touching unknown pin
  DDRC |= B00110100; // Setting SCL, SDA and GPS MOSFET pin as output
  DDRD &= B00110000; // 
  DDRD |= B00100000;

  PORTB |= B00111111; // Not touching crystal
  PORTC |= B01111111; // Not touching unknown pin
  PORTC &= B11001011; // Setting SCL, SDA and GPS MOSFET pin as LOW
  PORTD |= B11001111; //
  PORTD &= B11001111; // Set Clock power pin and Depth Multiplexer&MOSFET pin LOW
  
  // Disable ADC
  ADCSRA &= ~(1<<7);
  
  // Go to deep sleep
  SMCR |= (1<<2); // Power down mode
  SMCR |= 1; // Enable sleep
  
  // BOD disable
  MCUCR |= (3<<5); // Set both BODS and BODSE at the same time
  MCUCR = (MCUCR & ~(1<<5)) | (1<<6); // Then set the BODS bit and clear the BODSE bit at the same time
  __asm__ __volatile__("sleep"); // In line assembler sleep execute instruction
  delay(10);
  detachInterrupt(digitalPinToInterrupt(clockInterruptPin));
}

void wakeUpRoutine()
{
  PORTD |= B00010000; // Set Clock power pin high
  Wire.begin();
}

void goToSafeSleep(){
  bool fakeWakeup = true;
  while (fakeWakeup){
    goToSleep();
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
      printAlarm();
      delay(50);
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





int getDepth()
{
  String startMeasurements = "?M!";
  String getData = "?D0!";
  String SDI_Response = "";
  String SDI_Response_Stripped = "";
 
  delay(5000); // Needs five seconds to turn on
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
  Serial.println(SDI_Response);
  delay(5);
  SDI_Response = "";
  SDI_Response_Stripped = "";
  mySDI12.clearBuffer();
  delay(5);
  Serial.print(F("Depth = "));
  Serial.println(depth);
  
  return depth;
}
