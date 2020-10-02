/*
 * DataType.h
 *
 *  Created on: 02/mar/2015
 *      Author: Matteo
 */

#ifndef HEADER_DATATYPE_H_
#define HEADER_DATATYPE_H_

#define MAX_PUMP 7
#define TRUE  1
#define FALSE 0

struct PumpProperties{
	char Start_Hour;		//Ora di accensione della pompa
	char Start_Minute;	//Minuti di accensione della pompa
	char Stop_Hour;		//Ora di spegnimento della pompa
	char Stop_Minute;		//Minuti di spegnimento della pompa
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
};

struct SprinklerConfig
{
	PumpProperties Pump[MAX_PUMP];
	MotorProperties Motor;
	CurrentTime Time;
};



#endif /* HEADER_DATATYPE_H_ */
