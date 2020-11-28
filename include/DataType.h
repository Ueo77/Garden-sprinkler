/*
 * DataType.h
 *
 *  Created on: 02/mar/2015
 *      Author: Matteo
 */

#ifndef HEADER_DATATYPE_H_
#define HEADER_DATATYPE_H_

typedef char StringDisplay[17];

#define MAX_PUMP 7
#define TRUE  1
#define FALSE 0

#define LUNEDI		1
#define MARTEDI		2
#define MERCOLEDI	3
#define GIOVEDI		4
#define VENERDI		5
#define SABATO		6
#define DOMENICA	7

#define MASK_LUNEDI		0x01
#define MASK_MARTEDI	0x02
#define MASK_MERCOLEDI	0x04
#define MASK_GIOVEDI	0x08
#define MASK_VENERDI	0x10
#define MASK_SABATO		0x20
#define MASK_DOMENICA	0x40



struct PumpProperties{
	char Start_Hour;	//Ora di accensione della pompa
	char Start_Minute;	//Minuti di accensione della pompa
	char Stop_Hour;		//Ora di spegnimento della pompa
	char Stop_Minute;	//Minuti di spegnimento della pompa
	char Week;			//Giorni della settima in cui deve essere accesa la pompa: bit mask 0x01 LUNEDI
	char Enabled;
	char UseMotor;
	char PumpIO;
};

struct MotorProperties{
	unsigned short Pre_Start_Time;	//Tempo di preaccensione del motore
	char enabled;					//motore abilitato
};

struct CurrentTime {
	char hour;
	char minute;
	char second;
	char day;
	char month;
	unsigned int year;
};

struct SprinklerConfig
{
	PumpProperties Pump[MAX_PUMP];
	MotorProperties Motor;
	CurrentTime Time;
};

struct EthernetConfig
{
	/* data */
};



#endif /* HEADER_DATATYPE_H_ */
