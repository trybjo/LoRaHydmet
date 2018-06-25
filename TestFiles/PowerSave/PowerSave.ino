void setup() {
  // Code for temperature sensor
  Serial.begin(9600);
  Serial.println(F("BMP280 test"));
  
  if (!bme.begin()) {  
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    while (1);
  }
  //////////////////////////////////////

  
  //Disable ADC 
  ADCSRA &= ~(1 << 7); // Disables the AD-converter to save power
  
  //ENABLE SLEEP - this enables the sleep mode, if current mode is IDLE
  SMCR |= (3 << 1); //power-save mode
  SMCR |= 1;//enable sleep


  // Wake up: 
  // After Timer Overflow or Output Compare event if interrupt enable bits are set: TIMSK2
  // And global Interrupt Enable bit in SREG is set
  TIMSK2 |= (1 << 3); // Enables Output Compare Match B Interrupt
  ASSR |= (1 << 1); // Required to enable interrupt Asyncronous Status Register
  TCCR2B |= 7; // Select timer as CPU-clock devided by 1024
  
  
}

void loop() {
  // put your main code here, to run repeatedly:
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
  delay(100);
  //////////////////////////////////

  // Activating sleep: 
  MCUCR |= (3 << 5); //set both BODS and BODSE at the same time
  MCUCR = (MCUCR & ~(1 << 5)) | (1 << 6); //Clearing BODSE bit and set the BODS bit at the same time
  __asm__  __volatile__("sleep");//in line assembler to go to sleep
  
}
