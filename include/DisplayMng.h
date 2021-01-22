/*
 * DisplayMng.h
 *
 *  Created on: 02/mar/2015
 *      Author: Matteo
 */

#ifndef HEADER_DISPLAYMNG_H_
#define HEADER_DISPLAYMNG_H_

//#include <LiquidCrystal.h>

#include <LiquidCrystal_PCF8574.h>
#include <Wire.h>

#include "DataType.h"


void InitDisplay(SprinklerConfig *Cfg);

void ExecDisplay(bool Blink);

void Sub_Button(void);
void Main_Button(void);


#endif /* HEADER_DISPLAYMNG_H_ */
