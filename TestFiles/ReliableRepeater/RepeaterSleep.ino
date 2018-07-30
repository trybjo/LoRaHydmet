
void wakeUp(){  
  // Just a handler for the pin interrupt.
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
      delay(40);
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

void goToSleep(){
  
  attachInterrupt(digitalPinToInterrupt(wakeUpPin), wakeUp, FALLING);
  // Disable ADC
  ADCSRA &= ~(1<<7);
  
  // Go to deep sleep
  SMCR |= (1<<2); // Power down mode
  SMCR |= 1; // Enable sleep
  
  // BOD disable
  MCUCR |= (3<<5); // Set both BODS and BODSE at the same time
  MCUCR = (MCUCR & ~(1<<5)) | (1<<6); // Then set the BODS bit and clear the BODSE bit at the same time
  __asm__ __volatile__("sleep"); // In line assembler sleep execute instruction
  detachInterrupt(digitalPinToInterrupt(wakeUpPin));
}

