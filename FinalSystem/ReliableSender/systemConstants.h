/*
 * D4   -> Clock power
 * D5   -> Depth Multiplexer and MOSFET pin
 * 
 * D6   -> Power Humidity
 * 
 * A2   -> GPS MOSFET
 */
#define clockInterruptPin 3
#define SENDER_ADDRESS 2 // Range 1 to 4
#define depthDataPin A0
SDI12 mySDI12(depthDataPin); // Define the SDI-12 bus
Adafruit_BMP280 BMP;
Adafruit_AM2320 am2320 = Adafruit_AM2320(); // Instance for the humidity sensor.
#define humidityPower 6


/*--------------------------------------*
 * Change to the length of the message  *
 * -------------------------------------*/
#define messageLength 18

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

#define LoRa_FREQ 870.1

/*-----------------------------------------------------*
 * Sender and receiver address for the LoRa module     *
 * Also defines maximum packet size, 29 byte for data, *
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


/*---------------------------*
 * Set gpsEnable pin to A0   *
 *---------------------------*/
#define clockPowerPin 4
#define GPS_POWER 2 

#define INIT_SWITCH 8
