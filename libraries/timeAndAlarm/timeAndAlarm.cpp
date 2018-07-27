#include "timeAndAlarm.h"
#include <Arduino.h>
#include "RTClibExtended.h"
#include <Adafruit_Sensor.h>
#include <EEPROM.h>
//#include "pin_definitions.hpp"
#include <Wire.h>




timeAndAlarm::timeAndAlarm(RTC_DS3231 &RTC){
    _RTC = RTC;                                               // Instanciate the external clock                                               // Instanciate the GPS
}

long int timeAndAlarm::getTimeStamp(){
  DateTime timeNow = _RTC.now(); 
  // year = 2018, year%100 = 18
  long int timeInt = (long)timeNow.month() * 100000000 + (long)timeNow.day()*1000000 +((long)timeNow.year()%100)*10000;
  timeInt += (long)timeNow.hour()*100 + (long) timeNow.minute();
  return timeInt; 
}
long int timeAndAlarm::getTime(){
  DateTime timeNow = _RTC.now();
  return (long) timeNow.minute() * 100 + (long) timeNow.second();
}

 
// Returns positive value if the alarm is after the time now
// Returns negative value if the alarm is before the time now
long int timeAndAlarm::timeDifference(){
  DateTime timeNow = _RTC.now();
  int hourDiff = EEPROM.read(846) - timeNow.hour();
  int minuteDiff = EEPROM.read(847) - timeNow.minute();
  int secondDiff = EEPROM.read(848) - timeNow.second() + 60*minuteDiff + hourDiff*3600;
  return secondDiff;
}

void timeAndAlarm::setWakeUpPeriod(int wHour, int wMinute, int wSec){
  EEPROM.update(843, wHour);
  EEPROM.update(844, wMinute);
  EEPROM.update(845, wSec);
}

void timeAndAlarm::adjustWakeUpTime(long int seconds){
  // This should be able to handle negative and positive adjustments?
  int hours = 0;
  int minutes = 0;
  if(seconds>59){
    minutes = (int)seconds / 60;
    seconds = seconds - minutes * 60; 
  }

  int hh = EEPROM.read(846);
  int mm = EEPROM.read(847);
  int ss = EEPROM.read(848);
  
  updateAlarmTime(hh, mm, ss, hours, minutes, seconds);
}

void timeAndAlarm::setAlarm1(int period, int secondOffset){
  DateTime timeNow = _RTC.now();
  int minuteNow = timeNow.minute();
  int hourNow = timeNow.hour();
  
  if (period > 30){
    setAlarm1(hourNow+1, 0, secondOffset);
  }
  else if (secondOffset < 0 && minuteNow > 1){
    setAlarm1(timeNow.hour(), ((int)(minuteNow/period)+1)*period - 1, secondOffset + 60);
  }
  else if (minuteNow < 59-period){
    // We can add the period without reaching a new hour
    setAlarm1(timeNow.hour(), ((int)(minuteNow/period)+1)*period, secondOffset);
  }
  else {
    if (hourNow == 23){
      setAlarm1(0, 0, secondOffset);
    }
    else{
      setAlarm1(hourNow+1, 0, secondOffset);
    }
  }  
}

void timeAndAlarm::setAlarm1(int wHour, int wMinute, int wSec){
  EEPROM.update(846, wHour);
  EEPROM.update(847, wMinute);
  EEPROM.update(848, wSec);
  setAlarm1();
}

void timeAndAlarm::setAlarm1(){
  int hh = EEPROM.read(846);
  int mm = EEPROM.read(847);
  int ss = EEPROM.read(848);

  // setAlarm(Ds3231_ALARM_TYPES_t alarmType, byte seconds, byte minutes, byte hours, byte daydate)
  _RTC.setAlarm(ALM1_MATCH_HOURS, ss, mm, hh, 1); 
  _RTC.alarmInterrupt(1, true);
}

void timeAndAlarm::stopAlarm(){
  _RTC.armAlarm(1, false);
  _RTC.clearAlarm(1);
  _RTC.alarmInterrupt(1, false);
}

void timeAndAlarm::setNextWakeupTime(){
  // Updates the current wake-up time by adding period
  // Last alarm:
  int hh = EEPROM.read(846);
  int mm = EEPROM.read(847);
  int ss = EEPROM.read(848); 
  
  // Alarm period: 
  int periodHour = EEPROM.read(843);
  int periodMinute = EEPROM.read(844);
  int periodSecond = EEPROM.read(845);

  // Updating time for next alarm
  updateAlarmTime(hh, mm, ss, periodHour, periodMinute, periodSecond);
  
}

void timeAndAlarm::updateAlarmTime(int hh, int mm, int ss, int adjustHour, int adjustMinute, int adjustSecond){
  
  if (ss + adjustSecond > 59){
    // The aditional seconds causes us to add a minute
    EEPROM.update(848, ss + adjustSecond - 60);
    adjustMinute ++;
  }
  else if (ss + adjustSecond < 0){
    // The removal of seconds causes us to subtract minute
    EEPROM.update(848, ss + adjustSecond + 60);
    adjustMinute --;
  }
  else{
    // We can simply add the seconds
    EEPROM.update(848, ss + adjustSecond);
  }

  if (mm + adjustMinute > 59){
    // The added minute causes us to add an hour
    EEPROM.update(847, mm + adjustMinute - 60);
    adjustHour ++;
  }
  else if (mm + adjustMinute < 0){
    // The added minute causes us to subtract an hour
    EEPROM.update(847, mm + adjustMinute + 60);
    adjustHour --;
  }
  else{
    // We can simply add the minutes
    EEPROM.update(847, mm + adjustMinute);
  }

  if (hh + adjustHour > 23){
    // The added hour causes us to reach a new day
    EEPROM.update(846, hh + adjustHour - 24);
  }
  else if (hh + adjustHour < 0){
    // The added hour causes to go one day back
    EEPROM.update(846, hh + adjustHour + 24);
  }
  else{
    // We can simply add hours
    EEPROM.update(846, hh + adjustHour);
  }
}