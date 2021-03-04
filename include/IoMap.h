/*
 * IoMap.h
 *
 *  Created on: 02/mar/2015
 *      Author: Matteo
 */
#ifndef HEADER_IOMAP_H_
#define HEADER_IOMAP_H_


/*PUMP PIN OUT*/
#define PIN_PUMP1 2//8
#define PIN_PUMP2 3//9
#define PIN_PUMP3 4
#define PIN_PUMP4 5
#define PIN_PUMP5 6
#define PIN_PUMP6 7
#define PIN_PUMP7 8
#define PIN_MOTOR 9

/* ETHERNET RESERVED PIN*/
#define ETH_SS      10
#define ETH_MOSI    11
#define ETH_MISO    12
#define ETH_SCK     13


/*BUTTON PIN */
#define PIN_SCROLL_DOWN  14 //Analog 00
#define PIN_SCROLL_LEFT  15 //Analog 01
/*END BUTTON PIN */


#define RAIN_SENSOR 16//Analog 02
//Free A3

/*LCD PIN OUT*/
#define PIN_LCD_SDA 18//Analog 04
#define PIN_LCD_SCL 19//Analog 05
/*END LCD PIN OUT*/

/*END PUMP PIN OUT*/
#define CYCLE_TIME 100
#define BLINK_TIME 1000
#endif /* HEADER_IOMAP_H_ */

