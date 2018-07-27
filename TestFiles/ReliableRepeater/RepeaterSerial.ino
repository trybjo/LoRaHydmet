
void printPackageNumberMemory(int from){
  Serial.print(F("Package memory from "));
  Serial.print(from);
  Serial.println(F(": "));
  for (int i = 0; i < 4; i++){
    Serial.println(EEPROM.read(100*from+90+i)); 
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
  Serial.print(F("Received message from: "));
  Serial.print(from, HEX);
  Serial.print(F(" : "));
  Serial.println((int)message[0]);  
}
void printMemory(int from){
  Serial.print(F("Memory from "));
  Serial.print(from);
  Serial.println(F(":"));
  
  for (int i = 0; i < 8 ; i++){
    // i representing packet number
    if (EEPROM.read(100*i + 15*(from-1) + 11) != 0){
      Serial.print(F("For iterator value: "));
      Serial.print(i);
      Serial.print(F(" : "));
      Serial.println(EEPROM.read(100*i + 15*(from-1))); // The number that is in front of the message
    }    
  }
  Serial.println();
}
