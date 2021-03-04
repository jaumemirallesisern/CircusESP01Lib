/*
  	CircusESP01Lib.cpp  (Version 1.0.0)

	Implements the circusofthings.com API in Arduino devices when connected by wifi through an external ESP8266 module.

  	Created by Jaume Miralles Isern, June 26, 2019.
*/


#ifndef CircusESP01Lib_h
#define CircusESP01Lib_h

#include "Arduino.h"
#include <SoftwareSerial.h>

#define DEBUG_NO 0
#define DEBUG_YES 1
#define DEBUG_DEEP 2

class SoftwareSerial;

class CircusESP01Lib
{
  	public:
		CircusESP01Lib(SoftwareSerial *Serial1,int esp01BaudRate,char *server, char *token, char *ssid, char *pass, int debugLevel);
		void begin();
		void write(char *key, double value);
                double read(char *key);
	private:
		int mef();
                int checkTCP();
                int checkWIFI();
                int checkESP8266();
		SoftwareSerial *esp01Serial;
		int _esp01BaudRate;
		char *_server;
		char *_ssid;
                char *_pass;
		char *_token;
		int _debug;
		int counter;

		int comp(char *whole,char *target);
		int sendcommand(char *scom, char *dres, unsigned long timeout, int retries);
		int senddata(int socket, char *data, int length, char *dresp, unsigned long timeout, int retries);
		int setESP();
		int connectWIFI();
		int connectServer();
                int count(char *text);
                char* parseServerResponse(char *r, char *label, int offset);
                char* waitResponse(int timeout);
                void console(const __FlashStringHelper *message, int level);
                void console(char *message, int level);
                void console(char message, int level);
		void console(int message, int level);
};

#endif
