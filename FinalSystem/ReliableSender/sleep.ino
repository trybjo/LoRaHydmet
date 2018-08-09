

void setSleepConfig(){
  DDRB = B00000000;
  DDRC = B00111100;   // SCL and SDA  and power for GPS and LoRa as output
  DDRD = B01110000;   // Power for clock and depth Multiplexer&MOSFET set as output
  delay(100);
  PORTB = ~DDRB;      // Pull all in-ports high
  PORTC = ~DDRC;      // Pull all in-ports high, turn off power for GPS and LoRa, set SCL and SDA low
  PORTD = ~DDRD;      // Pull all in-ports high, turn off power for clock and depth sensor
  delay(200);
}

void setAwakePinConfig(){
  DDRB = B00111110;   // 
  DDRC = B00111100;
  DDRD = B01110000;  
  PORTB = ~DDRB;
  PORTC = ~DDRC;
  PORTD = ~DDRD;
}

void goToSleep()
{
  attachInterrupt(digitalPinToInterrupt(clockInterruptPin), wakeUpRoutine, LOW);
  // Disable ADC
  ADCSRA &= ~(1<<7);
  
  setSleepConfig();
  
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
  /*
  PORTD |= B00010000; // Set Clock power pin high
  Wire.begin();
  */
}

// Goes to sleep, and makes sure that only interrupts at the time of alarm wakes up the system
void goToSafeSleep(){
  bool fakeWakeup = true;
  while (fakeWakeup){
    goToSleep();
    
    activateClock();
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

