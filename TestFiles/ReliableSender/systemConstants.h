/*--------------------------------------------------------------------------------------------------------*
 * Pinout from Arduino to peripherals:                                                                    *
 * Reset  -> Pull high with 10K                                                                           *
 * GND    -> Humidity sensor pin 3 (from left, seen from side with holes)                                 *
 * 5V     -> Humidity sensor pin 1 (from left, seen from side with holes) + Temp&Press VIN + LoRa VIN     *   
 * RX/D0  -> GPS TX                                                                                       *
 * TX/D1  -> GPS RX                                                                                       *
 * D2     -> LoRa G0                                                                                      *
 * D3     -> Clock SQW                                                                                    *
 * D4     -> LoRa RST                                                                                     *
 * D5     ->                                                                                              *
 * D6     ->                                                                                              *
 * D7     ->                                                                                              *
 * D8     ->                                                                                              *
 * D9     ->                                                                                              *
 * D10    -> LoRa CS                                                                                      *
 * D11    -> LoRa MOSI                                                                                    *
 * D12    -> LoRa MISO                                                                                    *
 * D13    -> LoRa SCK                                                                                     *
 * A0     -> OTT (depth measure) signal cable (gray)                                                      *
 * A1     -> OTT (depth measure) MOSFETs                                                                  *
 * A2     ->                                                                                              *
 * A3     ->                                                                                              *
 * SDA/A4 -> Humidity sensor pin 2 (from left, seen from side with holes) + Clock SDA + Temp/Press SDI    *
 * SCL/A5 -> Humidity sensor pin 4 (from left, seen from side with holes) + Clock SCL + Temp/Press SCK    *
 *--------------------------------------------------------------------------------------------------------*/



/*-----------------------------------------------------*
 * Sender and receiver address for the LoRa module     *
 * Also defines maximum packet size, 22 byte for data, *
 * 1 byte for message length, 4 bytes header and 2 FCS *
 * Definition of RH_RF95_MAX_MESSAGE_LEN: RH_RF95.h    *
 * ----------------------------------------------------*/
 
#define RH_RF95_MAX_MESSAGE_LEN 29
#define SENDER_ADDRESS 1 // Range 1 to 4
#define RECEIVER_ADDRESS 5
#define REPEATER_ADDRESS 6


/*---------------------------------------------------*
 * Define pins for chip select, reset and interrupt  *
 * --------------------------------------------------*/
#define LoRa_CS 10
#define LoRa_RST 4
#define LoRa_INT 2
RH_RF95 lora(LoRa_CS, LoRa_INT); // Singleton instance of the radio driver.
RHReliableDatagram manager(lora, SENDER_ADDRESS); // Class to manage message delivery and receipt,using the lora declared above.

/*---------------------------------------------------*
 * Define pins for chip select, reset and interrupt  *
 * --------------------------------------------------*/
#define depthDataPin A0
#define depthMOSFETS A1
SDI12 mySDI12(depthDataPin); // Define the SDI-12 bus

/*----------------------------------------------*
 * The BMP sensor uses I2C for communication    *
 *----------------------------------------------*/

Adafruit_BMP280 BMP;

/*---------------------------*
 * Set gpsEnable pin to A2   *
 *---------------------------*/
#define gpsEnable A2 



/*---------------------------*
 * Set LoRa frequenzy [MHz]  *
 *---------------------------*/
#define LoRa_FREQ 868.0


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
#define clockInterruptPin 3

/*--------------------------------------*
 * Change to the length of the message  *
 * -------------------------------------*/
#define messageLength 21
