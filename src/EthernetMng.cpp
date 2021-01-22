/*
 * Ethernet.cpp
 *
 *  Created on: 28/nov/2020
 *      Author: Matteo
 */
#include "DataType.h"
#include "EthernetMng.h"
#include "IoMap.h"
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <Time.h>


#define NTP_PACKET_SIZE  48                     // Il time stamp NTP è nei primi 48 by del pacchetto udp

#define SERVER_REQUEST_CYCLE  65000          // Esegue una ricerca ogni 65536 cicli ovvero ogni 10 ore circa
#define WAIT_SERVER_ANSWER    10

#define SEND_TIME_REQUEST 10
#define DECODE_REQUEST    20
#define DECODE_RAW_DATA   30
#define DELIM " :"
extern UdpPumpCommand PumpCommand[MAX_PUMP];
extern bool PumpStarted [MAX_PUMP];

void GetPacket(void);
char packetBuffer[NTP_PACKET_SIZE]={0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };             // Buffer per gestire i pacchetti in ingresso ed uscita
void sendNTPpacket(const char * address);
void sendStatusPacket();

const char TimeServer[] = "it.pool.ntp.org";//"time.nist.gov";      // time.nist.gov NTP server
//const char WebServer[]  = "10.7.0.23";

extern StringDebug Debug;

// Istanza UDP necessaira per inviare e ricevere pacchetti UDP
EthernetUDP Udp;
byte UpdateTimeState=SEND_TIME_REQUEST;
byte sendStatus=0;
word Timer2 = 300;
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
unsigned int localPort = 8888;                  // Porta del servizio ntp

void InitEthernet(void)
{
 if (Ethernet.begin(mac) == 0) {
#ifdef SERIAL_DEBUG_ETH
    Serial.println("Failed to configure Ethernet using DHCP");
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    } else if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
#endif
    // no point in carrying on, so do nothing forevermore:
    while (true) 
    {
      delay(1);
    }
  }
  Udp.begin(localPort);

#ifdef SERIAL_DEBUG_ETH
      // print your local IP address:
      Serial.print("Ethernet done\n");
      Serial.println(Ethernet.localIP());
      Serial.println(Ethernet.gatewayIP());
      Serial.println(Ethernet.subnetMask());
      Serial.println(Ethernet.dnsServerIP());
#endif
}

void ExecEthernet(bool Blink)
{
 switch ( Ethernet.maintain() ) 
 {

    case 1:

      //renewed fail
#ifdef SERIAL_DEBUG_ETH
      Serial.println("Error: renewed fail");
#endif
      break;

    case 2:

      //renewed success
#ifdef SERIAL_DEBUG_ETH
      Serial.println("Renewed success");

      //print your local IP address:

      Serial.print("My IP address: ");

      Serial.println(Ethernet.localIP());
#endif
      break;

    case 3:

      //rebind fail
#ifdef SERIAL_DEBUG_ETH
      Serial.println("Error: rebind fail");
#endif
      break;

    case 4:

      //rebind success
#ifdef SERIAL_DEBUG_ETH
      Serial.println("Rebind success");

      //print your local IP address:

      Serial.print("My IP address: ");

      Serial.println(Ethernet.localIP());
#endif
      break;

    default:
    
      Ethernet.maintain();

#ifdef SERIAL_DEBUG_ETH
      Serial.println("Ethernet success");
      Serial.println(Ethernet.localIP());
      Serial.println(Timer2);
#endif
      if(Timer2 >0 )
      {
            Timer2--;
            GetPacket();
         
      } 
      else
      {
            switch (UpdateTimeState)
            {
            case SEND_TIME_REQUEST:
#ifdef SERIAL_DEBUG_ETH
                  Serial.println(F("ExecEthernet SEND_TIME_REQUEST"));
#endif
                  sendNTPpacket(TimeServer); // send an NTP packet to a time server
                  Timer2=WAIT_SERVER_ANSWER;
                  UpdateTimeState=DECODE_REQUEST;
                  break;

            case DECODE_REQUEST:
#ifdef SERIAL_DEBUG_ETH
                  Serial.println(F("ExecEthernet DECODE_REQUEST"));
#endif
                  GetPacket();
                  Timer2= SERVER_REQUEST_CYCLE;
                  UpdateTimeState=DECODE_RAW_DATA;
                  break;
            case DECODE_RAW_DATA:
#ifdef SERIAL_DEBUG_ETH
                  Serial.println(F("ExecEthernet DECODE_RAW_DATA"));
#endif
                  GetPacket();
                  Timer2= SERVER_REQUEST_CYCLE;
                  UpdateTimeState=SEND_TIME_REQUEST;
      
                  break;

            default:
#ifdef SERIAL_DEBUG_ETH
                  Serial.println(F("UpdataTimeState default"));
#endif
                  Timer2= SERVER_REQUEST_CYCLE;
                  UpdateTimeState=SEND_TIME_REQUEST;
                  break;
            }

      }
      
      
      break;

  }
}

// send an NTP request to the time server at the given address

void sendStatusPacket()
 {
      // set all bytes in the buffer to 0
      memset(packetBuffer, 0, NTP_PACKET_SIZE);
      // Initialize values needed to form NTP request
      // (see URL above for details on the packets)
      
      sprintf(packetBuffer,"%u%u%u%u%u%u%u",  PumpStarted[0],
                                              PumpStarted[1],   
                                              PumpStarted[2],
                                              PumpStarted[3],   
                                              PumpStarted[4],   
                                              PumpStarted[5],   
                                              PumpStarted[6]);   
      
      Serial.print(packetBuffer);
      // all NTP fields have been given values, now
      // you can send a packet requesting a timestamp:
      Udp.beginPacket(Udp.remoteIP(), Udp.remotePort()); // NTP requests are to port 123
      Udp.write(packetBuffer, NTP_PACKET_SIZE);
      Udp.endPacket();
}


// send an NTP request to the time server at the given address
void sendNTPpacket(const char * address)
 {
      // set all bytes in the buffer to 0
      memset(packetBuffer, 0, NTP_PACKET_SIZE);
      // Initialize values needed to form NTP request
      // (see URL above for details on the packets)
      packetBuffer[0] = 0b11100011;   // LI, Version, Mode
      packetBuffer[1] = 0;     // Stratum, or type of clock
      packetBuffer[2] = 6;     // Polling Interval
      packetBuffer[3] = 0xEC;  // Peer Clock Precision
      // 8 bytes of zero for Root Delay & Root Dispersion
      packetBuffer[12]  = 49;
      packetBuffer[13]  = 0x4E;
      packetBuffer[14]  = 49;
      packetBuffer[15]  = 52;

      // all NTP fields have been given values, now
      // you can send a packet requesting a timestamp:
      Udp.beginPacket(address, 123); // NTP requests are to port 123
      Udp.write(packetBuffer, NTP_PACKET_SIZE);
      Udp.endPacket();
}


void  GetPacket(void)
{     
      // wait to see if a reply is available
      if (Udp.parsePacket()) {
            // We've received a packet, read the data from it
            Udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

            // the timestamp starts at byte 40 of the received packet and is four bytes,
            // or two words, long. First, extract the two words:
            if(Udp.remotePort() == 123)
            {
                  unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
                  unsigned long lowWord  = word(packetBuffer[42], packetBuffer[43]);
                  // combine the four bytes (two words) into a long integer
                  // this is NTP time (seconds since Jan 1 1900):
                  unsigned long secsSince1900 = highWord << 16 | lowWord;
#ifdef SERIAL_DEBUG_ETH
                  int8_t NewHour,NewMinute,NewSecond;
                  unsigned long sourceport=Udp.remotePort();
                  Serial.print(F("Souce Port "));
                  Serial.println(sourceport);
                  Serial.print(F("Seconds since Jan 1 1900 = "));
                  Serial.println(secsSince1900);
                  // now convert NTP time into everyday time:
                  Serial.print(F("Unix time = "));
#endif
                  
                  // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
                  const unsigned long seventyYears = 2208985200UL;//2208988800UL;
                  // subtract seventy years:
                  unsigned long epoch = secsSince1900 - seventyYears ;
                  setTime(epoch);
#ifdef SERIAL_DEBUG_ETH
                  // print Unix time:
                  Serial.println(epoch);

                  NewHour     = ( (epoch  % 86400L) / 3600);
                  NewMinute   = (epoch  % 3600) / 60;
                  NewSecond   = epoch % 60;
                 
                  // print the hour, minute and second:
                  Serial.print("The Rome time is ");       // UTC is the time at Greenwich Meridian (GMT)
                  sprintf(Debug,"%.2u:%.2u:%.2u",NewHour,NewMinute,NewSecond);
                  Serial.println(Debug);
#endif
            }
            else
            {
#ifdef SERIAL_DEBUG_ETH
                  Serial.print(F("Content:"));
                  Serial.println(packetBuffer);                 
#endif
                  char *token;
                  int8_t word_number=0,PumpNumber=0,RelaysCmd=0;
                  bool FistNum;
    
                  token=strtok(packetBuffer, DELIM);
                  while(token)
                  {
                        if(strcmp(token,"open")==0 )
                        {
                              RelaysCmd   =-1;
                              FistNum     = TRUE;
                        }
                        else if(strcmp(token,"close") == 0 )
                        {
                              RelaysCmd   = 1;
                              FistNum     = TRUE;
                        }
                        else if(strcmp(token,"status") == 0 )
                        {
#ifdef SERIAL_DEBUG_ETH
                        Serial.println("SendStatus");
#endif
                              sendStatusPacket();
                        }
                        else
                        {
                              if(FistNum) 
                              {
                                    PumpNumber=atoi(token)-1;
                                    if (PumpNumber < MAX_PUMP && PumpNumber >= 0)
                                    {
                                          PumpCommand[PumpNumber].TurnOn = RelaysCmd;
                                          if(RelaysCmd == -1)
                                          {
                                                PumpCommand[PumpNumber].Delay=0;
                                                PumpCommand[PumpNumber].EnableCounter = FALSE;
                                          }  
                                    }
                                    FistNum = FALSE;
                              }
                              else
                              {
                                    PumpCommand[PumpNumber].Delay         = atoi(token)*1000;
                                    PumpCommand[PumpNumber].EnableCounter = TRUE;
                              }
                              
                        }

                        word_number++;
#ifdef SERIAL_DEBUG_ETH
                        sprintf(Debug,"Parola individuata: \'%s\'\n", token);
                        Serial.println(Debug);
#endif
                        token=strtok(NULL, DELIM);
                  }

            }
            
            
      }
      
      Ethernet.maintain();
}