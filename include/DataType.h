/*
 * DataType.h
 *
 *  Created on: 02/mar/2015
 *      Author: Matteo
 */

#ifndef HEADER_DATATYPE_H_
#define HEADER_DATATYPE_H_
#include <Arduino.h>

typedef char StringDisplay[17];
typedef char StringDebug[70];
typedef char StringCommand[5];

#define MAX_PUMP 7
#define TRUE  1
#define FALSE 0

#define DOMENICA	1
#define LUNEDI		2
#define MARTEDI		3
#define MERCOLEDI	4
#define GIOVEDI		5
#define VENERDI		6
#define SABATO		7


#define	MASK_DOMENICA	0x01
#define MASK_LUNEDI		0x02
#define MASK_MARTEDI	0x04
#define MASK_MERCOLEDI	0x08
#define MASK_GIOVEDI	0x10
#define MASK_VENERDI	0x20
#define MASK_SABATO		0x40



struct PumpProperties{
	byte Start_Hour=22;	//Ora di accensione della pompa
	byte Start_Minute=00;//Minuti di accensione della pompa
	byte Stop_Hour=22;		//Ora di spegnimento della pompa
	byte Stop_Minute=00;	//Minuti di spegnimento della pompa
	byte Week=0xff;				//Giorni della settima in cui deve essere accesa la pompa: bit mask 0x01 LUNEDI
	byte Enabled;			//Pompa abilitata
	byte UseMotor=TRUE;	//Richiede l'utilizzo del motore
	byte PumpIO;			//Numero di IO cui è associata
};

struct MotorProperties{
	unsigned short Pre_Start_Time=1;	//Tempo di preaccensione del motore
	byte enabled=TRUE;								//motore abilitato
};

struct CurrentTime {
	byte hour;
	byte minute;
	byte second;
	byte day;
	byte month;
	unsigned int year;
};

struct SprinklerConfig
{
	PumpProperties Pump[MAX_PUMP];
	MotorProperties Motor;
	CurrentTime Time;
};

struct UdpPumpCommand
{
	int8_t  TurnOn;
	uint32_t Delay;	
	uint8_t EnableCounter;	


};

//#define MAIN_SERIAL_DEBUG 			// Abilita il debug seriale sul main necessario per i debug seriale
//#define SERIAL_DEBUG_ETH  			// Abilita il debug seriale ethernet
//#define SERIAL_DEBUG_CONFIG_WRITER	// Abilita il debug della configurazione
//#define SERIAL_DEBUG_DISPLAY		// Abilita il debug del display

#endif /* HEADER_DATATYPE_H_ */
