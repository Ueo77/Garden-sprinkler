/*
 * displayMng.cpp
 *
 *  Created on: 02/mar/2015
 *      Author: Matteo
 */
#include <Arduino.h>
#include "DisplayMng.h"
#include "IoMap.h"
#include "ConfigWriter.h"
#include <Time.h>

#define PAGE_PUMP1	0   /*Pompa numero 1*/
#define PAGE_PUMP2	1   /*Pompa numero 2*/
#define PAGE_PUMP3	2   /*Pompa numero 3*/
#define PAGE_PUMP4	3   /*Pompa numero 4*/
#define PAGE_PUMP5	4   /*Pompa numero 5*/
#define PAGE_PUMP6	5   /*Pompa numero 6*/
#define PAGE_PUMP7	6   /*Pompa numero 7*/
#define PAGE_MOTOR	7   /*Gestione motore*/
#define PAGE_TIME	8   /*Orologio*/
#define PAGE_RESET	9	/*Pagina di reset*/

#define MAX_PAGES PAGE_RESET

#define SUB_PUMP_PAGE_START_HOUR   1
#define SUB_PUMP_PAGE_START_MINUTE 2
#define SUB_PUMP_PAGE_STOP_HOUR    3
#define SUB_PUMP_PAGE_STOP_MINUTE  4
#define SUB_PUMP_PAGE_ENABLE_MOTOR 5

#define SCREEN_COL 16
#define SCREEN_ROW 2

#define BOUNCE_DURATION 400
volatile unsigned long bounceTime=0,bounceTimesSub=0;

char blink;
static void Sub_Button(void);
static void Main_Button(void);

/*Variabili interne*/
LiquidCrystal lcd(PIN_LCD_RS, PIN_LCD_EN, PIN_LCD_D4, PIN_LCD_D5, PIN_LCD_D6, PIN_LCD_D7);
SprinklerConfig *Config;


volatile byte CurrentPage;
volatile byte CurrentSubPage;
volatile byte MaxCurrentSubPage;
char Saved=0;

//char lcdString [17];
StringDisplay lcdString;
char dbgString[100];
/*******************/

/*Funzioni ad uso interno*/
void increase(int Main, int Sub);
void ResetPage(void);

void CleanPage(void);
void WeekPage(PumpProperties * Pump,char Blink,byte Day);
void PumpPage(PumpProperties * Pump);
void DayOfWeekDecode(int day, char *DayName);
void TimePage(CurrentTime * Time);
void MotorPage(MotorProperties *Motor);
/************************/

void InitDisplay(SprinklerConfig *Cfg)
{
	/* definisco il numero di colonne 16 e righe 2*/
	lcd.begin(SCREEN_COL, SCREEN_ROW);
	/*Faccio una puttanata all'avvio*/

	attachInterrupt(INT_SCROLL_LEFT, Main_Button, RISING);
	attachInterrupt(INT_SCROLL_DOWN, Sub_Button, RISING);

	CurrentPage=PAGE_TIME;
	CurrentSubPage=0;
	//Copia interna della configurazione dell'irrigatore
	Config=Cfg;
	Serial.println("Init LCD\n");

}
void CleanPage(void)
{
	lcd.setCursor(0, 0);
	lcd.print("               ");
	lcd.setCursor(0, 1);
	lcd.print("               ");
}

void WeekPage(PumpProperties * Pump,char Blink,byte Day)
{
	char L=0,Ma=0,Me=0,G=0,V=0,S=0,D=0,empty;
	lcd.setCursor(0,0);
	lcd.print("  L M M G V S D  ");
	lcd.setCursor(0,1);
	blink  ? empty=' ' : empty='#';
	if( Day == LUNEDI)
	 	(Pump->Week & MASK_LUNEDI) 		? L ='*': L =empty; 
	else 
		(Pump->Week & MASK_LUNEDI) 		? L ='*': L =' '; 

	if( Day == MARTEDI)
		(Pump->Week & MASK_MARTEDI) 	? Ma='*': Ma=empty;
	else 
		Ma = ' '; 

	if( Day == MERCOLEDI)
		(Pump->Week & MASK_MERCOLEDI)	? Me='*': Me=empty;
	else 
		Me = ' ';

	if( Day == GIOVEDI)
		(Pump->Week & MASK_GIOVEDI) 	? G ='*': G =empty;
	else 
		G = ' '; 

	if( Day == VENERDI)
		(Pump->Week & MASK_VENERDI) 	? V ='*': V =empty;
	else 
		V = ' '; 

	if( Day == SABATO)
		(Pump->Week & MASK_SABATO) 		? S ='*': S =empty;		
	else 
		S = ' '; 

	if( Day == DOMENICA)
		(Pump->Week & MASK_DOMENICA) 	? D ='*': D =empty;
	else 
		D = ' '; 

	sprintf(lcdString,"  %c %c %c %c %c %c %c ",L,Ma,Me,G,V,S,D);

	lcd.print(lcdString);
	Serial.print("WeekPage = ");
	Serial.print(Day);
	Serial.print("Week = ");
	Serial.print(Pump->Week);
	Serial.print(lcdString);
	Serial.print("\n");

}
void PumpPage(PumpProperties * Pump)
{
	char EnableMot = '_';
	if (CurrentSubPage <=6 )
	{
		lcd.setCursor(0, 0);
		if ( CurrentSubPage == 0 ) //In visualizzazzione
			sprintf(lcdString,"Pompa #%.1u        ",CurrentPage+1);
		else 					//In programmazione aggiungo un asterisco
			sprintf(lcdString,"Pompa #%.1u       *",CurrentPage+1);

		lcd.print (lcdString);
		lcd.setCursor(0, 1);
	}
	Serial.print("PumpPage ");
	Serial.print(Pump->UseMotor,DEC);
	Serial.print("\n");
	if(Pump->UseMotor == 1)
		EnableMot = 'x';
	if( CurrentSubPage == 0 )
	{
	sprintf(lcdString,"I%.2d:%.2dF%.2d:%.2d M:%c",  Pump->Start_Hour,
			                                        Pump->Start_Minute,
													Pump->Stop_Hour,
													Pump->Stop_Minute,
													EnableMot);
	}
	else
	{
		if (blink == TRUE)
		{
			switch (CurrentSubPage)
			{
			case 1:
				sprintf(lcdString,"I  :%.2dF%.2d:%.2d M:%c",	Pump->Start_Minute,
																Pump->Stop_Hour,
																Pump->Stop_Minute,
																EnableMot);
				break;
			case 2:
				sprintf(lcdString,"I%.2d:  F%.2d:%.2d M:%c",	Pump->Start_Hour,
															   	Pump->Stop_Hour,
															   	Pump->Stop_Minute,
															   	EnableMot);
				break;
			case 3:
				sprintf(lcdString,"I%.2d:%.2dF  :%.2d M:%c", 	 Pump->Start_Hour,
																	Pump->Start_Minute,
																	Pump->Stop_Minute,
																	EnableMot);
				break;
			case 4:
				sprintf(lcdString,"I%.2d:%.2dF%.2d:   M:%c",  Pump->Start_Hour,
																	Pump->Start_Minute,
																	Pump->Stop_Hour,
																	EnableMot);
				break;
			case 5:
				sprintf(lcdString,"I%.2d:%.2dF%.2d:%.2d M: ",  Pump->Start_Hour,
																	Pump->Start_Minute,
																	Pump->Stop_Hour,
																	Pump->Stop_Minute);
				break;
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
			case 11:
			case 12:
			case 13:
				WeekPage(Pump,blink,CurrentSubPage - 5);
				break;
			}
		}
		else
			if (CurrentSubPage <6 ) 
			{
				sprintf(lcdString,"I%.2d:%.2dF%.2d:%.2d M:%c",  Pump->Start_Hour,
						                                        Pump->Start_Minute,
																Pump->Stop_Hour,
																Pump->Stop_Minute,
																EnableMot);
			}	
			else
			{
				WeekPage(Pump,blink,CurrentSubPage - 5);
			}
			
		blink=!blink;
	}
	lcd.print (lcdString);
	Serial.print("Motor Enable ");
	Serial.println(Pump->UseMotor,DEC);
	Serial.print("\n");
}

void DayOfWeekDecode(int day, char *DayName)
{
	switch (day)
	{
	case LUNEDI:
		sprintf(DayName,"%s","Lun ");
		break;
	case MARTEDI:
		sprintf(DayName,"%s","Mar ");
		break;
	case MERCOLEDI:
		sprintf(DayName,"%s","Merc");
		break;
	case GIOVEDI:
		sprintf(DayName,"%s","Giov");
		break;
	case VENERDI:
		sprintf(DayName,"%s","Ven ");
		break;
	case SABATO:
		sprintf(DayName,"%s","Sab ");
		break;
	case DOMENICA:
		sprintf(DayName,"%s","Dom ");
		break;
	default:
		sprintf(DayName,"%s","Opss....");
		break;
	}			
}

void TimePage(CurrentTime * Time)
{
	int WeekDay=0;
	
	WeekDay=weekday();
	DayOfWeekDecode(WeekDay,lcdString);
	
	if (CurrentSubPage == 0) //In visualizzazzione
	{
		lcd.setCursor(0, 0);
		sprintf(lcdString,"%s %2.d/%2.d/%4.d ",lcdString,day(),month(),year());
	}
	
		
	lcd.print(lcdString);
	
	if (CurrentSubPage!=0)
	{
		if (blink == TRUE)
		{
			switch (CurrentSubPage)
			{
				case 1:
					lcd.setCursor(0, 1);
					sprintf(lcdString,"        :%.2u:%.2u  ",Time->minute,Time->second);
					break;
				case 2:
					lcd.setCursor(0, 1);
					sprintf(lcdString,"      %.2u:  :%.2u  ",Time->hour,Time->second);
					break;
				case 3:
					lcd.setCursor(0, 1);
					sprintf(lcdString,"      %.2u:%.2u:    ",Time->hour,Time->minute);
					break;
				case 4:
					lcd.setCursor(0, 0);
					sprintf(lcdString,"     %2.d/%2.d/%4.d ",day(),month(),year());
					break;
				case 5:
					lcd.setCursor(0, 0);
					sprintf(lcdString,"%s   /%2.d/%4.d ",lcdString,month(),year());	
					break;
				case 6:
					lcd.setCursor(0, 0);
					sprintf(lcdString,"%s %2.d/  /%4.d ",lcdString,day(),year());
					break;
				case 7:
					lcd.setCursor(0, 0);
					sprintf(lcdString,"%s %2.d/%2.d/     ",lcdString,day(),month());
					break;
			}
		}
		else
		{
			switch (CurrentSubPage)
			{	
				
				case 1:
				case 2:
				case 3:
					lcd.setCursor(0, 1);
					sprintf(lcdString,"      %.2u:%.2u:%.2u  ",Time->hour,Time->minute,Time->second);
					break;
				case 4:
				case 5:
				case 6:
				case 7:
					lcd.setCursor(0, 0);
					//In programmazione aggiungo un asterisco
					sprintf(lcdString,"%s %2.d/%2.d/%4.d",lcdString,day(),month(),year());
					break;
			}
		}
		blink=!blink;
	}
	else
		{
			lcd.setCursor(0, 1);
			sprintf(lcdString,"      %.2u:%.2u:%.2u  ",hour(),minute(),second());
		}
		

	lcd.print (lcdString);
}

void MotorPage(MotorProperties *Motor)
{
	char EnableMot = '_';
	lcd.setCursor(0, 0);

	if (CurrentSubPage ==0) //In visualizzazzione
		sprintf(lcdString,"     Motore     ");
	else 					//In programmazione aggiungo un asterisco
		sprintf(lcdString,"     Motore    *");

	lcd.print (lcdString);
	lcd.setCursor(0, 1);
	if(Motor->enabled)
		EnableMot = 'x';
	if (CurrentSubPage!=0)
	{
		if (blink == TRUE)
		{
			switch (CurrentSubPage)
			{
				case 1:
				case 2:
				case 3:
					sprintf(lcdString,"Antic       En:%c", EnableMot);
				break;
				case 4:
					sprintf(lcdString,"Antic%.3ds   En: ", Motor->Pre_Start_Time);
				break;
			}
		}
		else
		{
			sprintf(lcdString,"Antic%.3ds   En:%c", Motor->Pre_Start_Time, EnableMot);
		}

		blink=!blink;
	}
	else
	{
		sprintf(lcdString,"Antic%.3ds   En:%c", Motor->Pre_Start_Time, EnableMot);
	}
	lcd.print (lcdString);

}

void ResetPage(void)
{
	lcd.setCursor(0, 0);
	if (CurrentSubPage ==0) //In visualizzazzione
		sprintf(lcdString,"     Reset      ");
	else 					//In programmazione aggiungo un asterisco
		sprintf(lcdString,"     Reset    *");
	lcd.print (lcdString);

	lcd.setCursor(0, 1);
	if (Saved == FALSE )
		sprintf(lcdString," Tasto 2 salva  ");
	else
		sprintf(lcdString," Salvato!       ");
	lcd.print (lcdString);
}
void Increase(int Main, int Sub)
{
	sprintf(dbgString,"Increase: Main %d Sub %d",Main,Sub);
	Serial.println(dbgString);
	Saved = FALSE;
	switch (Main)
	{
		case PAGE_PUMP1:
		case PAGE_PUMP2:
		case PAGE_PUMP3:
		case PAGE_PUMP4:
		case PAGE_PUMP5:
		case PAGE_PUMP6:
		case PAGE_PUMP7:
			switch (Sub)
			{
				case 1:
					Config->Pump[Main].Start_Hour++;
					if (Config->Pump[Main].Start_Hour > 23)
						Config->Pump[Main].Start_Hour=0;
					break;
				case 2:
					Config->Pump[Main].Start_Minute++;
					if (Config->Pump[Main].Start_Minute > 59)
						Config->Pump[Main].Start_Minute=0;
					break;
				case 3:
					Config->Pump[Main].Stop_Hour++;
						if (Config->Pump[Main].Stop_Hour > 23)
							Config->Pump[Main].Stop_Hour=0;
					break;
				case 4:
					Config->Pump[Main].Stop_Minute++;
					if (Config->Pump[Main].Stop_Minute > 59)
						Config->Pump[Main].Stop_Minute=0;
					break;
				case 5:
					Config->Pump[Main].UseMotor=!Config->Pump[Main].UseMotor;
					break;
				case 6:	
					Config->Pump[Main].Week  = ((Config->Pump[Main].Week & MASK_LUNEDI)	 ? (Config->Pump[Main].Week & ~MASK_LUNEDI) : (Config->Pump[Main].Week | MASK_LUNEDI) );
				case 7:	
					Config->Pump[Main].Week  = ((Config->Pump[Main].Week & MASK_MARTEDI)	 ? (Config->Pump[Main].Week & ~MASK_MARTEDI) : (Config->Pump[Main].Week | MASK_MARTEDI) );
				case 8:	
					Config->Pump[Main].Week  = ((Config->Pump[Main].Week & MASK_MERCOLEDI)	 ? (Config->Pump[Main].Week & ~MASK_MERCOLEDI) : (Config->Pump[Main].Week | MASK_MERCOLEDI) );
				case 9:	
					Config->Pump[Main].Week  = ((Config->Pump[Main].Week & MASK_GIOVEDI)	 ? (Config->Pump[Main].Week & ~MASK_GIOVEDI) : (Config->Pump[Main].Week | MASK_GIOVEDI) );
				case 10:	
					Config->Pump[Main].Week  = ((Config->Pump[Main].Week & MASK_VENERDI)	 ? (Config->Pump[Main].Week & ~MASK_VENERDI) : (Config->Pump[Main].Week | MASK_VENERDI) );
				case 11:	
					Config->Pump[Main].Week  = ((Config->Pump[Main].Week & MASK_SABATO)	 ? (Config->Pump[Main].Week & ~MASK_SABATO) : (Config->Pump[Main].Week | MASK_SABATO) );
				case 12:	
					Config->Pump[Main].Week  = ((Config->Pump[Main].Week & MASK_DOMENICA)	 ? (Config->Pump[Main].Week & ~MASK_DOMENICA) : (Config->Pump[Main].Week | MASK_DOMENICA) );
			}

			break;
		case PAGE_MOTOR:
			switch(Sub)
			{
				case 1:
					Config->Motor.Pre_Start_Time+=100;
					break;
				case 2:
					Config->Motor.Pre_Start_Time+=10;
					break;
				case 3:
					//Non c'e' bisgno di controllare raggiunto il massimo si resetta da solo
					Config->Motor.Pre_Start_Time+=1;
					break;
				case 4:
					Config->Motor.enabled=!Config->Motor.enabled;
					sprintf(dbgString,"PAGE_MOTOR: Config->Motor.enabled %d",Config->Motor.enabled);
					Serial.println(dbgString);
					break;
			}
			//Superati 1000 sec riporto a 0
			if(Config->Motor.Pre_Start_Time > 999)
				Config->Motor.Pre_Start_Time=0;

		break;
		case PAGE_TIME:
			switch(Sub)
			{
				case 1:
					Config->Time.hour++;
					if(Config->Time.hour>23)
						Config->Time.hour=0;
					break;
				case 2:
					Config->Time.minute++;
					if(Config->Time.minute>59)
						Config->Time.minute=0;
					break;
				case 3:
					Config->Time.second=0;
					break;
			}
		break;
		case PAGE_RESET:
			switch(Sub)
			{
				case 1:
					sprintf(dbgString,"PAGE_RESET: Save data");
					Serial.println(dbgString);
					SaveData(Config);
					Saved =TRUE;
					break;

			}

	}

}


void Sub_Button(void )
{
	if (abs(millis() - bounceTimesSub) > BOUNCE_DURATION)
	{
		bounceTimesSub = millis();  // set whatever bounce time in ms is appropriate
		//Sono nella root delle pagine e devo l'utente vuole entrare
		if(CurrentSubPage == 0)
		{
			Serial.println("CurrentSubPage == 0");
			CurrentSubPage=1;
		}
		else
		{
			Serial.println("CurrentSubPage != 0");
			Increase(CurrentPage,CurrentSubPage);
		}
		sprintf(dbgString,"Sub button: CurrentSubPage %d CurrentPage %d",CurrentSubPage,CurrentPage);
		Serial.println(dbgString);
	}
}
void Main_Button(void )
{

	if (abs(millis() - bounceTime) > BOUNCE_DURATION)
	{
	    // Your code here to handle new button press ?
	    bounceTime = millis();  // set whatever bounce time in ms is appropriate

	    //Se non � stato selezionato niente allora prosegui sulle pagine
	    if(CurrentSubPage==0)
	    {
	    	Serial.println("CurrentSubPage=0");
	    	CurrentPage++;
	    	if(CurrentPage > MAX_PAGES)
			{
				CurrentPage=0;
			}
	    		
	    }
	    else
	    {
	    	Serial.println("CurrentSubPage!=0");
	    	CurrentSubPage++;
	    	if(CurrentSubPage > MaxCurrentSubPage)
	    	{
	    		CurrentPage++;
	    		if(CurrentPage > MAX_PAGES)
				{
					CurrentPage=0;
				}
	    			
	    		CurrentSubPage=0;
	    	}
	    }
		sprintf(dbgString,"Main button: CurrentSubPage %d CurrentPage %d",CurrentSubPage,CurrentPage);
		Serial.println(dbgString);
	}

}


void ExecDisplay(void)
{
	// Non ho capito perchè ma queste due righe impediscono il reset della variabile 
	//sprintf(dbgString,"ExecDisplay: CurrentSubPage %d",CurrentSubPage);
	//Serial.println(dbgString);
	switch(CurrentPage)
	{
		case PAGE_PUMP1:
		case PAGE_PUMP2:
		case PAGE_PUMP3:
		case PAGE_PUMP4:
		case PAGE_PUMP5:
		case PAGE_PUMP6:
		case PAGE_PUMP7:
			PumpPage(&(Config->Pump[CurrentPage]));
			MaxCurrentSubPage = 13; //Definisco il numero di sotto pagine del display
			break;
		case PAGE_MOTOR:
			MotorPage(&(Config->Motor));
			MaxCurrentSubPage = 4; //Definisco il numero di sotto pagine del display
			break;
		case PAGE_TIME:
			TimePage(&(Config->Time));
			MaxCurrentSubPage = 7; // Definisco il numero di sotto pagine del display
			break;
		case PAGE_RESET:
			ResetPage();
			MaxCurrentSubPage = 0;// Definisco il numero di sotto pagine del display
			break;
	}
	sprintf(dbgString,"ExecDisplay: CurrentSubPage %d CurrentPage %d MaxCurrentSubPage %d",CurrentSubPage,CurrentPage,MaxCurrentSubPage);
	Serial.println(dbgString);
}
