


/*-------------------------------------------------------------------------------------------------------*
 * Pinout from Arduino to peripherals:                                                                   *
 * GND  -> Humidity sensor pin 3 (from left, seen from side with holes)                                  *
 * 5V   -> Humidity sensor pin 1 (from left, seen from side with holes) + Temp&Press VIN + LoRa VIN      *
 *                                                                                                       *
 * A0   -> UNIK 500 (see sketch on github for connection)                                                *
 *                                                                                                       *
 * D2   -> LoRa G0                                                                                       *
 * D3   -> LoRa RST                                                                                      *
 *                                                                                                       *
 * D5   -> Rwmp&Press SDI (aka. MOSI)                                                                    *
 * D6   -> Temp&Press SDO (aka. MISO)                                                                    *
 * D7   -> Temp&Press SCK                                                                                *
 * D8   -> Temp&Press CS                                                                                 *
 * D9   -> MOSFET Gate                                                                                   *
 * D10  -> LoRa CS                                                                                       *
 * D11  -> LoRa MOSI                                                                                     *
 * D12  -> LoRa MISO                                                                                     *
 * D13  -> LoRa SCK        
 * 
 * A0   -> gpsEnable                                                                              *
 *                                                                                                       *
 *                                                                                                       *
 * SDA  -> Humidity sensor pin 2 (from left, seen from side with holes)                                  *
 * SCL  -> Humidity sensor pin 4 (from left, seen from side with holes)                                  *
 *-------------------------------------------------------------------------------------------------------*/

#define RH_RF95_MAX_MESSAGE_LEN 29
#define RECEIVER_ADDRESS 5
#define REPEATER_ADDRESS 6


/*---------------------------------------------------*
 * Define pins for chip select, reset and interrupt  *
 * --------------------------------------------------*/
#define LoRa_CS 10
#define LoRa_RST 4
#define LoRa_INT 2
#define LoRa_FREQ 868.0

/*----------------------------------------------*
 * Define, MOSI, MISO, SCK and chip celect pin  *
 *----------------------------------------------*/
#define BMP_MOSI 5
#define BMP_MISO 6
#define BMP_SCK 7
#define TempAndPressure_CS 8

/*---------------------------*
 * Set gpsEnable pin to A0   *
 *---------------------------*/
#define gpsEnable 14 



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
#define wakeUpPin 2

/*--------------------------------------*
 * Change to the length of the message  *
 * -------------------------------------*/
#define messageLen 22
