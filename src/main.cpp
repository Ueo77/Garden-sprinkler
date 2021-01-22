// Do not remove the include below
#include <Time.h>
#include <TimeAlarms.h>
#include <Arduino.h>
#include <Wire.h>
#include "Irrigatore.h"
#include "ConfigWriter.h"
#include "IoMap.h"
#include "DataType.h"
#include "DisplayMng.h"
#include "EthernetMng.h"

//Configurazione sistema di irrigazione
SprinklerConfig Configurazione;
//Identifictivo di pompa accesa
bool PumpStarted [MAX_PUMP];
//Numero di elettrovalvole accese
short NumberOfStartedPump;
//Identificativo di motore pozzo acceso
bool MotorOn;
bool Blink;
uint16_t BlinkTimer=0;
UdpPumpCommand PumpCommand[MAX_PUMP];

StringDebug Debug;
//The setup function is called once at startup of the sketch
void PumpStart(short int PumpID, SprinklerConfig *Conf);
void PumpStop(short int PumpID,SprinklerConfig *Conf);
bool CheckTime (TimeElements PumpTime,TimeElements  Now);
bool CheckWorkingDay(byte WeekMask, TimeElements  Now);
void CheckPump(TimeElements *PumpTime,TimeElements *Now, PumpProperties PumpConfig[]);
void MotorStop ();


void setup()
{
	time_t  Clock;
	TimeElements Clock_TM;
	uint8_t i;
	
	//Apre la porta seriale a 9600 bps
#ifdef MAIN_SERIAL_DEBUG
	Serial.begin(9600);
#endif
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
	BlinkTimer= 0;
	
	//Inizializzo l'orologio con la data salvata
	setTime(Clock);

	//inizializzo il display
	InitDisplay(&Configurazione);
	//Inizializzo la rete ethernet
	InitEthernet();
	//Numero di elettropvalvole accese
	NumberOfStartedPump=0; 

#ifdef MAIN_SERIAL_DEBUG
	Serial.println(" ");
	Serial.println("Setup done\n");
#endif	
	
} 

// The loop function is called in an endless loop
void loop()
{
	TimeElements PumpTime,Now;
	


	Main_Button();
	Sub_Button();
	
	if (BlinkTimer < BLINK_TIME)
	{
		BlinkTimer+=CYCLE_TIME;
		//Gestisco ethernet e NTP
		ExecEthernet(Blink); 
	}
	else
	{
		BlinkTimer=0;
	
		breakTime(now(),Now);
		//Avvio eventuali utenze
		CheckPump(&PumpTime,&Now,Configurazione.Pump);

		ExecDisplay(Blink);

		//Gestisco il display ed i tasti
	
		Blink=!Blink;
	}

	//verifico ogni 1000 milliscondi perchè devo gestire il display
	delay(CYCLE_TIME);

}

//Funzione per accendere'elettrovalvola corrizpondente
//nel caso sia richiesto preaccende il motore a meno che non sia gia' acceso
void PumpStart(short int PumpID,SprinklerConfig *Conf)
{
	#ifdef MAIN_SERIAL_DEBUG
	//	sprintf(Debug,"UseMotor %d Motor.enabled %d MotorOn %d",Conf->Pump[PumpID].UseMotor,Conf->Motor.enabled,MotorOn);
		Serial.println(Conf->Pump[PumpID].UseMotor);
		Serial.println(Conf->Motor.enabled);
		Serial.println(MotorOn);
	#endif
	//if((Conf->Pump[PumpID].UseMotor == TRUE) && (Conf->Motor.enabled==TRUE) && MotorOn == FALSE)
	//{
		digitalWrite(PIN_MOTOR, HIGH);
	//	delay(Conf->Motor.Pre_Start_Time*1000);
		MotorOn = TRUE;
	//}
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
	if((PumpTime.Hour == Now.Hour) && (PumpTime.Minute == Now.Minute) )
		return TRUE;
	else
		return FALSE;
}

//Deve restituire true se il giorno della settimana in noe
bool CheckWorkingDay(byte WeekMask, TimeElements  Now)
{ 
	bool ret;
	byte Day;
	Day = 1 << Now.Wday;
	//sprintf(dbgString, "CheckWorkingDay  0x%02x- %d",Day,Now.Wday);
	//Serial.println(dbgString);
	ret = WeekMask && Day;
	return ret;
}

void CheckPump(TimeElements *PumpTime,TimeElements *Now, PumpProperties PumpConfig[])
{
	byte i;

	for (i=0; i < MAX_PUMP; i++)
	{
		//Verifico se devo accendere la pompa
		PumpTime->Hour    = PumpConfig[i].Start_Hour;
		PumpTime->Minute  = PumpConfig[i].Start_Minute;
		if ( CheckWorkingDay(PumpConfig[i].Week,*Now) )  
		{
			//Accendo la pompa se non e' gia' accesa e se e' configurata che si deve accendere
			if (( CheckTime(*PumpTime, *Now) == TRUE )  && ( PumpStarted[i] != TRUE ) )
			{
				PumpStart(i,&Configurazione);
				PumpStarted[i]=TRUE; 
				NumberOfStartedPump++;
#ifdef MAIN_SERIAL_DEBUG
				sprintf(Debug, "Avviata la pompa numero %d NumberOfStartedPump %d",i,NumberOfStartedPump);
				Serial.println(Debug);
#endif
			}
		
		
			//Verifico se devo spegnere la pompa
			PumpTime->Hour   = PumpConfig[i].Stop_Hour;
			PumpTime->Minute = PumpConfig[i].Stop_Minute;
		
			if (( CheckTime(*PumpTime, *Now) == TRUE ) && ( PumpStarted[i] == TRUE ) )
			{
				PumpStop(i,&Configurazione);
				PumpStarted[i]=FALSE;
				NumberOfStartedPump--;
#ifdef MAIN_SERIAL_DEBUG
				sprintf(Debug, "Avviata la pompa numero %d NumberOfStartedPump %d",i,NumberOfStartedPump);
				Serial.println(Debug);

#endif
			}
		}

		if(PumpCommand[i].TurnOn == 1 )
		{
			if( PumpStarted[i]== FALSE )
			{
				PumpStart(i,&Configurazione);
				PumpStarted[i]=TRUE;
				NumberOfStartedPump++;
			}

			if(PumpCommand[i].EnableCounter == TRUE)
			{
				PumpCommand[i].Delay-= BLINK_TIME;
				if(PumpCommand[i].Delay <= 0)
				{
						PumpCommand[i].EnableCounter=FALSE;
						PumpCommand[i].Delay=0;
						PumpCommand[i].TurnOn =-1;


				}
			}
			
		
		}

		if(PumpCommand[i].TurnOn == -1 )
		{
			  PumpCommand[i].TurnOn=0;
				PumpStop(i,&Configurazione);
				PumpStarted[i]=FALSE;
				NumberOfStartedPump--;
		}

	}
	
	//Se non ci sono elettrovalvole accese spengo il motore
	if ( NumberOfStartedPump <= 0 )
	{
	    MotorStop();
	    //Per sicurezza fiswo il numero di pompe accese a 0 ma non dovrebbe servire a niente
	    NumberOfStartedPump=0;
	}
}