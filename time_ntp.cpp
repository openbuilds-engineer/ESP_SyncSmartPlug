/*-----------------------------------------------------------
NTP & time routines for ESP8266
    for ESP8266 adapted Arduino IDE

by Stefan Thesen 05/2015 - free for anyone

code for ntp adopted from Michael Margolis
code for time conversion based on http://stackoverflow.com/
-----------------------------------------------------------*/

// note: all timing relates to 01.01.2000

#include "time_ntp.h"

// NTP specifics
IPAddress timeServer(196, 21, 187, 2); // time.nist.gov NTP server
WiFiUDP udp;  // A UDP instance to let us send and receive packets over UDP
unsigned int ntpPort = 2390;          // local port to listen for UDP packets
const int NTP_PACKET_SIZE = 48;       // NTP time stamp is in the first 48 bytes of the message
byte tPacketBuffer[ NTP_PACKET_SIZE];  //buffer to hold incoming and outgoing packets


static unsigned short days[4][12] =
{
    {   0,  31,  60,  91, 121, 152, 182, 213, 244, 274, 305, 335},
    { 366, 397, 425, 456, 486, 517, 547, 578, 609, 639, 670, 700},
    { 731, 762, 790, 821, 851, 882, 912, 943, 974,1004,1035,1065},
    {1096,1127,1155,1186,1216,1247,1277,1308,1339,1369,1400,1430},
};


unsigned int date_time_to_epoch(date_time_t* date_time)
{
    unsigned int second = date_time->second;  // 0-59
    unsigned int minute = date_time->minute;  // 0-59
    unsigned int hour   = date_time->hour;    // 0-23
    unsigned int day    = date_time->day-1;   // 0-30
    unsigned int month  = date_time->month-1; // 0-11
    unsigned int year   = date_time->year;    // 0-99
    return (((year/4*(365*4+1)+days[year%4][month]+day)*24+hour)*60+minute)*60+second;
}


unsigned long getNTPTimestamp()
{
  unsigned long ulSecs2000;

  udp.begin(ntpPort);
  sendNTPpacket(timeServer); // send an NTP packet to a time server
  delay(2000);    // wait to see if a reply is available
  int cb = udp.parsePacket();
  if(!cb)
  {
    sendNTPpacket(timeServer); // send an NTP packet to a time server
    delay(3000);    // wait a bit longer
    cb = udp.parsePacket();
  }
  if(!cb)
  {
    Serial.println("Timeserver not accessible! - No RTC support!");
    ulSecs2000=0;
  }
  else
  {
    Serial.print("packet received, length=");
    Serial.println(cb);
    udp.read(tPacketBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:
    unsigned long highWord = word(tPacketBuffer[40], tPacketBuffer[41]);
    unsigned long lowWord = word(tPacketBuffer[42], tPacketBuffer[43]);

    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    ulSecs2000  = highWord << 16 | lowWord;
    ulSecs2000 -= 2208988800UL; // go from 1900 to 1970
    ulSecs2000 -= 946684800UL; // go from 1970 to 2000
    ulSecs2000 += 7200UL; // +2 hours
  }
  return(ulSecs2000);
}


// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
{
  Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(tPacketBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  tPacketBuffer[0] = 0b11100011;   // LI, Version, Mode
  tPacketBuffer[1] = 0;     // Stratum, or type of clock
  tPacketBuffer[2] = 6;     // Polling Interval
  tPacketBuffer[3] = 0xEC;  // Peer Clock Precision

  // 8 bytes of zero for Root Delay & Root Dispersion
  tPacketBuffer[12]  = 49;
  tPacketBuffer[13]  = 0x4E;
  tPacketBuffer[14]  = 49;
  tPacketBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(tPacketBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}


void epoch_to_date_time(date_time_t* date_time,unsigned int epoch)
{
    date_time->second = epoch%60; epoch /= 60;
    date_time->minute = epoch%60; epoch /= 60;
    date_time->hour   = epoch%24; epoch /= 24;

    unsigned int years = epoch/(365*4+1)*4; epoch %= 365*4+1;

    unsigned int year;
    for (year=3; year>0; year--)
    {
        if (epoch >= days[year][0])
            break;
    }

    unsigned int month;
    for (month=11; month>0; month--)
    {
        if (epoch >= days[year][month])
            break;
    }

    date_time->year  = years+year;
    date_time->month = month+1;
    date_time->day   = epoch-days[year][month]+1;
}

unsigned int seconds_since_midnight(unsigned int epoch){
  date_time_t date_time;
  epoch_to_date_time(&date_time,epoch);
  date_time.second = 0;
  date_time.minute = 0;
  date_time.hour   = 0;

  return epoch - date_time_to_epoch(&date_time);
}

String epoch_to_string(unsigned int epoch)
{
  date_time_t date_time;
  epoch_to_date_time(&date_time,epoch);
  String s;
  int i;

  s = date_time.hour;
  s+= ":";
  i=date_time.minute;
  if (i<10) {s+= "0";}
  s+= i;
  s+= ":";
  i=date_time.second;
  if (i<10) {s+= "0";}
  s+= i;
  s+= " - ";
  s+= date_time.day;
  s+= ".";
  s+= date_time.month;
  s+= ".";
  s+= 2000+(int)date_time.year;

  return(s);
}
