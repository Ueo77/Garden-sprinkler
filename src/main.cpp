// Do not remove the include below
#include "Irrigatore.h"
#include "ConfigWriter.h"
#include "IoMap.h"
#include "DataType.h"
#include "DisplayMng.h"

#include <Time.h>
#include <TimeAlarms.h>
#include <Arduino.h>

//Configurazione sistema di irrigazione
SprinklerConfig Configurazione;
//Identifictivo di pompa accesa
bool PumpStarted [MAX_PUMP];
//Numero di elettrovalvole accese
short NumberOfStartedPump;
//Identificativo di motore pozzo acceso
bool MotorOn;
extern char dbgString[100];

//The setup function is called once at startup of the sketch
void PumpStart(short int PumpID, SprinklerConfig *Conf);
void PumpStop(short int PumpID,SprinklerConfig *Conf);
bool CheckTime (TimeElements PumpTime,TimeElements  Now);
void MotorStop ();


void setup()
{
	time_t  Clock;
	TimeElements Clock_TM;
	uint8_t i;
	//Apre la porta seriale a 9600 bps
	Serial.begin(9600);
	memset(&Configurazione,0,sizeof(Configurazione));

	LoadData(&Configurazione);
	//Associo i pin alle rispettive pompe
	Configurazione.Pump[0].PumpIO=PIN_PUMP1;
	Configurazione.Pump[1].PumpIO=PIN_PUMP2;
	Configurazione.Pump[2].PumpIO=PIN_PUMP3;
	Configurazione.Pump[3].PumpIO=PIN_PUMP4;
	Configurazione.Pump[4].PumpIO=PIN_PUMP5;
	Configurazione.Pump[5].PumpIO=PIN_PUMP6;
	Configurazione.Pump[6].PumpIO=PIN_PUMP7;
	//configuro i pin dei relay come output
	for (i=0;i<MAX_PUMP;i++)
	{
		pinMode(Configurazione.Pump[i].PumpIO,OUTPUT);
	}
	pinMode(PIN_MOTOR,OUTPUT);

	Clock_TM.Hour=Configurazione.Time.hour;
	Clock_TM.Minute=Configurazione.Time.minute;
	Clock_TM.Second=0;


	Clock=makeTime(Clock_TM);
	//Inizializzo l'orologio con la data salvata
	setTime(Clock);

	//inizializzo il display
	InitDisplay(&Configurazione);
	//Numero di elettropvalvole accese
	NumberOfStartedPump=0;
	Serial.println(" ");
	Serial.println("Setup done\n");
	

}

// The loop function is called in an endless loop
void loop()
{
	time_t ttNow;
	TimeElements tePumpTime,teNow;
	short int i;

	ttNow = now();
	breakTime(ttNow,teNow);


	for (i=0; i < MAX_PUMP; i++)
	{
		//Verifico se devo accendere la pompa
		tePumpTime.Hour    = Configurazione.Pump[i].Start_Hour;
		tePumpTime.Minute  = Configurazione.Pump[i].Start_Minute;
		
		//Accendo la pompa se non e' gia' accesa e se e' configurata che si deve accendere
		if (( CheckTime(tePumpTime, teNow) == TRUE )  && ( PumpStarted[i] != TRUE ) )
		{
			PumpStart(i,&Configurazione);
			PumpStarted[i]=TRUE;
			NumberOfStartedPump++;
			sprintf(dbgString, "Avviata la pompa numero %d NumberOfStartedPump %d",i,NumberOfStartedPump);
			Serial.println(dbgString);
		}
		
		//Verifico se devo spegnere la pompa
		tePumpTime.Hour   = Configurazione.Pump[i].Stop_Hour;
		tePumpTime.Minute = Configurazione.Pump[i].Stop_Minute;
		
		if (( CheckTime(tePumpTime, teNow) == TRUE ) && ( PumpStarted[i] == TRUE ) )
		{
					PumpStop(i,&Configurazione);
					PumpStarted[i]=FALSE;
					NumberOfStartedPump--;
					sprintf(dbgString, "Avviata la pompa numero %d NumberOfStartedPump %d",i,NumberOfStartedPump);
					Serial.println(dbgString);
		}

	}
	
	//Se non ci sono elettrovalvole accese spengo il motore
	if ( NumberOfStartedPump <= 0 )
	{
	    MotorStop();
	    //Per sicurezza fiswo il numero di pompe accese a 0 ma non dovrebbe servire a niente
	    NumberOfStartedPump=0;
	}

	//Gestisco il display ed i tasti

	ExecDisplay();
	//salvo lo stato dell'orologio
	//SaveTime();
	//verifico ogni 200 milliscendi perche devo gestire il display
	delay(1000);

}

//Funzione per accendere'elettrovalvola corrizpondente
//nel caso sia richiesto preaccende il motore a meno che non sia gia' acceso
void PumpStart(short int PumpID,SprinklerConfig *Conf)
{
	if((Conf->Pump[PumpID].UseMotor = TRUE) && (Conf->Motor.enabled==TRUE) && MotorOn == FALSE)
	{
		digitalWrite(PIN_MOTOR, HIGH);
		delay(Conf->Motor.Pre_Start_Time*1000);
		MotorOn = TRUE;
	}
	digitalWrite(Conf->Pump[PumpID].PumpIO,HIGH);
}


//Funzione per spegenere l'elettrovalvola
void PumpStop(short int PumpID, SprinklerConfig *Conf)
{
	digitalWrite(Conf->Pump[PumpID].PumpIO,LOW);
}

//Funzione per spegnere il motore
void MotorStop()
{
 digitalWrite(PIN_MOTOR, LOW);
 MotorOn = FALSE;
}

//Funzione per verificare se dati due elements time sono uguali
bool CheckTime (TimeElements PumpTime, TimeElements Now )
{
	if((PumpTime.Hour == Now.Hour) && (PumpTime.Minute == Now.Minute))
		return TRUE;
	else
		return FALSE;
}
