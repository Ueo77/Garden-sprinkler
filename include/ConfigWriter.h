/*
 * ConfigWriter.hpp
 *
 *  Created on: 12/mar/2015
 *      Author: Matteo
 */

#ifndef HEADER_CONFIGWRITER_HPP_
#define HEADER_CONFIGWRITER_HPP_
#include "DataType.h"

void SaveData(SprinklerConfig *Cfg);
void LoadData(SprinklerConfig *Cfg);
void SaveTime();

#endif /* HEADER_CONFIGWRITER_HPP_ */
