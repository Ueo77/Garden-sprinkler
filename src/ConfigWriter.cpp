/*
 * ConfigWriter.c
 *
 *  Created on: 12/mar/2015
 *      Author: Matteo
 */
#include <EEPROM.h>
#include <Arduino.h>
#include "Time.h"
#include "DataType.h"
#ifdef SERIAL_DEBUG_CONFIG_WRITER
extern StringDebug Debug;
#endif
void SaveData(SprinklerConfig *Cfg)
{
	int  addr=0;
	byte Low,Hight,PumpID=0;
	for(PumpID=0;PumpID<MAX_PUMP;PumpID++)
	{
		EEPROM.write(addr++,Cfg->Pump[PumpID].Enabled);
		EEPROM.write(addr++,Cfg->Pump[PumpID].Start_Hour);
		EEPROM.write(addr++,Cfg->Pump[PumpID].Start_Minute);
		EEPROM.write(addr++,Cfg->Pump[PumpID].Stop_Hour);
		EEPROM.write(addr++,Cfg->Pump[PumpID].Stop_Minute);
		EEPROM.write(addr++,Cfg->Pump[PumpID].UseMotor);
		EEPROM.write(addr++,Cfg->Pump[PumpID].Week);
	}
	//Configurazione motore
	//Estraggo la parte bassa
	Low=(byte)(Cfg->Motor.Pre_Start_Time & 0x00ff);
	//Estraggo la parte alta
	Hight=(byte)((Cfg->Motor.Pre_Start_Time & 0xff00)>>8);
	EEPROM.write(addr++,Low);
	EEPROM.write(addr++,Hight);
	EEPROM.write(addr++,Cfg->Motor.enabled);
	//Configurazione Orologio
	EEPROM.write(addr++,Cfg->Time.hour);
	EEPROM.write(addr++,Cfg->Time.minute);
	EEPROM.write(addr++,Cfg->Time.second);
#ifdef SERIAL_DEBUG_CONFIG_WRITER
	sprintf(Debug,"SaveData: Cfg->Motor.Pre_Start_Time %x Low %x Hight %x addr%d",Cfg->Motor.Pre_Start_Time,Low,Hight,addr);
	Serial.println(Debug);
#endif
}

void LoadData(SprinklerConfig *Cfg)
{
	int  addr=0;
	byte Low,Hight,PumpID=0;

	for(PumpID=0;PumpID<MAX_PUMP;PumpID++)
	{
		Cfg->Pump[PumpID].Enabled	 	= EEPROM.read(addr++);
		Cfg->Pump[PumpID].Start_Hour 	= EEPROM.read(addr++);
		Cfg->Pump[PumpID].Start_Minute	= EEPROM.read(addr++);
		Cfg->Pump[PumpID].Stop_Hour		= EEPROM.read(addr++);
		Cfg->Pump[PumpID].Stop_Minute	= EEPROM.read(addr++);
		Cfg->Pump[PumpID].UseMotor		= EEPROM.read(addr++);
		Cfg->Pump[PumpID].Week			= EEPROM.read(addr++);
	}
	Low=EEPROM.read(addr++);
	Hight=EEPROM.read(addr++);
	//Configurazione motore
	Cfg->Motor.Pre_Start_Time = (0x00ff & (short)Low) | ((short)(Hight) << 8);
	Cfg->Motor.enabled		  = EEPROM.read(addr++);
	//Configurazione Orologio
	Cfg->Time.hour			  = EEPROM.read(addr++);
	Cfg->Time.minute		  = EEPROM.read(addr++);
	Cfg->Time.second		  = EEPROM.read(addr++);
#ifdef SERIAL_DEBUG_CONFIG_WRITER
	sprintf(Debug,"LoadData done");
	Serial.println(Debug);
#endif
 }


void SaveTime()
{
	int  addr=45;
	//Configurazione Orologio
	EEPROM.write(addr++,hour());
	EEPROM.write(addr++,minute());
	EEPROM.write(addr++,second());
}
