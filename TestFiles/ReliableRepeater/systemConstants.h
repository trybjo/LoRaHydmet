


/*-------------------------------------------------------------------------------------------------------*
 * Pinout from Arduino to peripherals:                                                                   *
 * GND  -> Humidity sensor pin 3 (from left, seen from side with holes)                                  *
 * 5V   -> Humidity sensor pin 1 (from left, seen from side with holes) + Temp&Press VIN + LoRa VIN      *
 *                                                                                                       *
 * A0   ->                                                 *
 * A1   ->                                                                                               *
 * A2   -> GPS power switch                                                                 *
 * A3   -> LoRa power switch                                                                               
 
 * D1                                                                                                      *
 * D2   -> LoRa G0                                                                                       
 * D3   -> Clock interrupt                                                                               *
 * D4   -> clockPowerPin                                                                                      *
 *                                                                                                       *
 * D5   ->                                                                    *
 * D6   ->                                                               *
 * D7   ->                                                                                *
 * D8   -> INIT_BUTTON                                                                                *
 * D9   -> LoRa RST                                                                                   *
 * D10  -> LoRa CS                                                                                       *
 * D11  -> LoRa MOSI                                                                                     *
 * D12  -> LoRa MISO                                                                                     *
 * D13  -> LoRa SCK                                                                                      * 
 *                                                                                                       *
 *                                                                                                       
 *                                                                                                       * 
 * RX   -> GPS TX                                                                                                      
 * TX   -> GPS RX                                                                                        *
 * SDA  -> Clock SDA                                                                                     *
 * SCL  -> Clock SCL                                                                                     *
 *-------------------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------*
 *  Pin set to high causes the system to boot in startup-mode *
 *------------------------------------------------------------*/
#define INIT_BUTTON 8

/*-----------------------------------------------------*
 * Sender and receiver address for the LoRa module     *
 * Also defines maximum packet size, 22 byte for data, *
 * 1 byte for message length, 4 bytes header and 2 FCS *
 * Definition of RH_RF95_MAX_MESSAGE_LEN: RH_RF95.h    *
 * ----------------------------------------------------*/
 
#define RH_RF95_MAX_MESSAGE_LEN 29
#define RECEIVER_ADDRESS 5
#define REPEATER_ADDRESS 6


/*---------------------------------------------------*
 * Define pins for chip select, reset and interrupt  *
 * --------------------------------------------------*/
#define LoRa_CS 10
#define LoRa_RST 9
#define LoRa_INT 2
#define LoRa_POWER 3


/*-----------------------------------------------------*
 * digital pin set high to supply current to the clock *
 *-----------------------------------------------------*/
#define clockPowerPin 4

/*---------------------------*
 * Set gpsEnable pin to A0   *
 *---------------------------*/
#define GPS_POWER 2 

#define LoRa_FREQ 870.1

/*---------------------------------------------------------------------------------------------------------------*
 *                                       Water level sensor                                                      *
 *                                                                                                               *
 * The water level sensor don't need any preparation                                                             *
 * BUT THE MEASURING RANGE MUST BE FIGURED OUT.                                                                  *
 * THE RESISTOR VALUE AND THEREFORE THE MAPPING VALUES MUST BE CHOSEN THEREAFTER.                                *
 * Higher resistor value -> higher resolution but lower measuring range.                                         *
 * The microcontroller can be damaged if the resistor value is too high and the sensor is too deep in the water. *
 * Max depth -> 20mA. Max voltage to microcontroller = 5V. V=RI -> R=250 ohm for max depth.                      *
 * Higher resistor values can be used if only lower water levels are needed.                                     *
 *---------------------------------------------------------------------------------------------------------------*/

/* ----------------------------------------------------------------------------------------------*
 * Set spreading factor, 6 ~ 12.                                                                 *
 * LoRa is a Chirp Spread Spectrum (CSS) modulation.                                             *
 * This setting determines the number of chirps per bit of data.                                 *
 * At the highest value (SF 12), the link has higher range and is more robust,                   *
 * but the data rate is lowered significantly.                                                   *
 * The lowest value (SF 6) is a special case designed for the highest possible data rate.        *
 * In this mode however, the length of the packet has to be known in advance.                    *
 * Also, CRC check is disabled.                                                                  *
 *-----------------------------------------------------------------------------------------------*/
#define spreadingFactor 12

/*-----------------------------------------------------------------------------------------------*
 * Set bandwidth, option: 7800,10400,15600,20800,31250,41700,62500,125000,250000,500000 [Hz].    *
 * Lower BandWidth for longer distance. Semtech recommends to keep it above or equal to 62.5 kHz *
 * to be compatible with the clock speed.                                                        *
 *-----------------------------------------------------------------------------------------------*/
#define signalBandwidth 125E3

/*-----------------------------------------------------------------------------------------------*
 * Set coding rate, 5 ~ 8 (values are actually 4/x).                                             *
 * LoRa modem always performs forward error correction.                                          *
 * The amount of these error checks is expressed by the coding rate.                             *
 * The lowest value (CR 4/5) results in higher data rate and less robust link,                   *
 * the highest value (CR 4/8) provides more robust link at the expense of lowering data rate.    *
 *-----------------------------------------------------------------------------------------------*/
#define codingRateDenominator 8

/*------------------------------*
 * Using pin 2 as wake up pin   *
 *------------------------------*/
#define wakeUpPin 3

/*--------------------------------------*
 * Change to the length of the message  *
 * -------------------------------------*/
#define messageLength 22
