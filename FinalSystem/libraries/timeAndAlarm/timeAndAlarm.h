
/*----------------------------------------------------------------------------------------------*
 * This library uses memory positions 843-858 and reads and writes them with the EEPROM library *
 * For compatability, these memory positions must not be used by other parts of the code        *
 *----------------------------------------------------------------------------------------------*/


class RTC_DS3231;
class DateTime;


class timeAndAlarm{
    public: 
        /*-------------------------------------------------------------------------*
         * Intialze our local variable, a pointer, to point to the argument RTC    * 
         * This way, when we change the instance here, we also change the instance *
         * in the script that instanciates timeAndAlarm                            *
         *-------------------------------------------------------------------------*/
        timeAndAlarm(RTC_DS3231 &RTC);

        /*------------------------------------------------------*
         * Returns the RTC-time on the format: (m)mddyyhhMM     *
         *------------------------------------------------------*/
        long int getTimeStamp();

        /*----------------------------------------------------*
         * Returns the RTC-time on the format: MMss           *
         * This function was made for debugging the code      *
          *---------------------------------------------------*/
        long int getTime();

        /* ------------------------------------------------------------------------*
         * Calculates the difference between now and time of alarm                 *
         * Returns positive value if the alarm is after the time now               *
         * Returns negative value if the alarm is before the time now              *
         * Negative time difference means the alarm has gone off already           *
         * As the time since alarm increases, the difference becomes more negative *
         * As the time until alarm decreases, the difference becomes less positive *
         * ------------------------------------------------------------------------*/
        long int timeDifference();

        /*----------------------------------------------------------------------------------*
         * If the input value is positive, changes the alarm time forward 'seconds' seconds *
         * If input value is negative, changes the alarm time backwards 'seconds' seconds   *
         *----------------------------------------------------------------------------------*/
        void adjustWakeUpTime(long int seconds);

        /*--------------------------------------------------------------*
         * Sets alarm to a multiple of 'period' with reference minute 0 *
         * Sets the alarm to the next time instance for that period     *
         * 'period' must be in minutes                                  *
         * if 'period' > 30, sets alarm to minute 0 at next hour        * 
         * Only works for abs(secondOffset) < 60                        *
         *--------------------------------------------------------------*/
        void setAlarm1(int period, int secondOffset);

        /*---------------------------------------------------------*
         * Writes the arguments to memory positions for alarm time *
         * Sets the alarm from these memory positions              *
         *---------------------------------------------------------*/
        void setAlarm1(int wHour, int wMinute, int wSec);

        /*-------------------------------------------*
         * Reads from memory and sets the alarm with *
         * hour, minutes and seconds as arguments    *
         *-------------------------------------------*/
        void setAlarm1();

        /*------------------------------------------------------------------*
         * Stops Alarm1 by clearing and disarming alarm through RTC library *
         *------------------------------------------------------------------*/
        void stopAlarm();

        /*---------------------------------------------*
         * Writes to memory positions for alarm period *
         *---------------------------------------------*/
        void setWakeUpPeriod(int wHour, int wMinute, int wSec);

        /*-----------------------------------------------------*
         * Reads last alarm time and alarm period from memory  *
         * Sums hours, minutes and seconds and wites to memory *
         * positions for the alarm time                        *
         *-----------------------------------------------------*/
        void setNextWakeupTime();
        
        
        
    private: 
        /*------------------------------------------------*
         * &_RTC is a reference to an instance of the RTC *
         *------------------------------------------------*/
        RTC_DS3231 &_RTC;

        /*---------------------------------------------------------------------*
         * Adds 'adjust' values to the other arguments, and writes to memory   *
         * positions for the alarm time                                        *
         * 'adjust' values can be both positive and negative                   *
         *                                                                     *
         * Makes sure that the alarm is adjusted by 'adjust' values, and takes *
         * care of change of days, hours and minutes                           *
         *---------------------------------------------------------------------*/
        void updateAlarmTime(int hh, int mm, int ss, int adjustSecond, int adjustMinute, int adjustHour);
};