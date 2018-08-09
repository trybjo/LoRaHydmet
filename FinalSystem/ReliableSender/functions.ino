// This function reads the position data of the GPS module and stores in in EEPROM position 5-11
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
      // Writing the new position data to memory 
      EEPROM.update(j + 5, positionData[j]);
    }
}

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


void storeInEEPROM(uint8_t data[], int dataLen)
{
 for(int i=0; i<dataLen; i++)
 {
  EEPROM.update(data[0]*100+i, data[i]);
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


// Returns temperature [*C] with two decimals precision.
long int getTemperature()
{
  delay(50);
  long int temp = (BMP.readTemperature() * 100);
  Serial.print(F("Temp: "));
  Serial.println(temp);
  delay(10);
  return temp;
}

// Returns pressure [Pa].
long int getPressure()
{
  delay(50);
  long int pressure = BMP.readPressure();
  delay(10);
  return pressure;
}


long int getHumidity()
{
  long int humidity = (am2320.readHumidity() * 10 );
  Serial.print(F("Humidity: "));
  Serial.println(humidity);
  delay(10);
  return humidity;
}
