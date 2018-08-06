
void wakeUp(){  
  // Wake up precedure
  activateClock();
  
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
      delay(200);
    }
  }
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

void powerNap1(int n){
  cli();
  MCUSR = 0;
  WDTCSR = (1<<WDCE)|(1<<WDE);
  WDTCSR = 0;
  WDTCSR = (1<<WDIE) | (1<<WDP3) | (1<<WDP0); // Set for interrupt mode
  sei();

}
// Just a handler for the interrupt
ISR(WDT_vect) {
  MCUSR = 0;
  wdt_disable();
  /* ReEnable the watchdog interrupt,                                     *
   * as this gets reset when entering this ISR and automatically enables  *
   *the WDE signal that resets the MCU the next time the  timer overflows */
  //WDTCSR |= (1<<WDIE);

}


// Puts the system to sleep for 'n' multiples of 8 seconds
void powerNap(int n){
  for (int i = 0; i < n; i++){
    //MCUSR &= ~(1 << 3);  // Clear WDRF, Watchdog system reset flag

    wdt_enable(WDTO_8S); // watchdog timer
    SREG |= (1 << 7);       // Set global interrupt mode
    WDTCSR |= (1 << 4);   // Set WDCE, Watchdog Change Enable
    WDTCSR = B01110001;     // Set watchdog to 'Interrupt mode', 
    sei(); // Enable all interrupts
    
    // Disable ADC
    ADCSRA &= ~(1<<7);
    
    // Go to deep sleep
    SMCR |= (1<<2); // Power down mode
    SMCR |= 1; // Enable sleep
    
      
    // BOD disable
    MCUCR |= (3<<5); // Set both BODS and BODSE at the same time
    MCUCR |= (1<<6);
    MCUCR &= ~(1 << 5);
    //MCUCR = (MCUCR & ~(1<<5)) | (1<<6); // Then set the BODS bit and clear the BODSE bit at the same time
    __asm__ __volatile__("sleep"); // In line assembler sleep execute instruction
  }
}

void goToSleep(){
  
  attachInterrupt(digitalPinToInterrupt(wakeUpPin), wakeUp, FALLING);
  // Disable ADC
  ADCSRA &= ~(1<<7);

  // Set all possible pins as input, pull them high
  // Set all outports low
  setSleepConfig();
  delay(100); // Needed in order to set output low, and pull high
  
  // Go to deep sleep
  SMCR |= (1<<2); // Power down mode
  SMCR |= 1; // Enable sleep
  
  // BOD disable
  MCUCR |= (3<<5); // Set both BODS and BODSE at the same time
  MCUCR = (MCUCR & ~(1<<5)) | (1<<6); // Then set the BODS bit and clear the BODSE bit at the same time
  __asm__ __volatile__("sleep"); // In line assembler sleep execute instruction
  detachInterrupt(digitalPinToInterrupt(wakeUpPin));
}
void setAwakePinConfig(){
  DDRB = B00111110;
  DDRC = B00111100;
  DDRD = B00000000;  
  PORTB = ~DDRB;
  PORTC = ~DDRC;
  PORTD = ~DDRD;
}

void setSleepConfig(){
  DDRB = B00000000;
  DDRC = B00111100;   // Power for GPS and LoRa, SCL and SDA all set as output
  DDRD = B00010000;   // Power for clock set as output
  PORTB = ~DDRB;      // Pull all in-ports high
  PORTC = ~DDRC;      // Pull all in-ports high, turn off power for GPS and LoRa turn SCL and SDA low
  PORTD = ~DDRD;      // Pull all in-ports high, turn off power for clock
}

































