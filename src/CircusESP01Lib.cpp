/*
  	CircusESP01Lib.cpp  (Version 1.0.0)

	Implements the circusofthings.com API in Arduino devices when connected by wifi through an external ESP8266 module.

  	Created by Jaume Miralles Isern, June 26, 2019.
*/


#include "CircusESP01Lib.h"
#include <SoftwareSerial.h>


CircusESP01Lib::CircusESP01Lib(SoftwareSerial *Serial1, int esp01BaudRate, char *server, char *token, char *ssid, char *pass, int debugLevel)
{
	_debug = debugLevel;
	_ssid = ssid;
        _pass = pass;
	_server = server;
	_token = token;
	esp01Serial = Serial1;
	_esp01BaudRate = esp01BaudRate;
}

void CircusESP01Lib::console(char message, int level) {
    if(level<=_debug) {
        Serial.print(message);
    }
}

void CircusESP01Lib::console(int message, int level) {
    if(level<=_debug) {
        Serial.print(message);
    }
}

void CircusESP01Lib::console(char *message, int level) {
    if(level<=_debug) {
        Serial.print(message);
    }
}

void CircusESP01Lib::console(const __FlashStringHelper *message, int level) {
    if(level<=_debug) {
        Serial.print(message);
    }
}

int CircusESP01Lib::sendcommand(char *scom, char *dresp, unsigned long timeout, int retries) {
    char rresp[200];
    for( int i = 0; i < sizeof(rresp);  ++i )
        rresp[i] = (char)0;
    for(int i=0;i<retries;i++){
        esp01Serial->print(scom);
        delay(50);
        unsigned long startTime = millis();
        int j=0;
        while (millis()-startTime < timeout) {
            while (esp01Serial->available()) {
                char c = esp01Serial->read();
                if(c!='\0') {rresp[j] = c;} else {rresp[j] = ' ';} // para más inri los huecos son \0
		console(c,2);
                j++;
                if (strstr(rresp,dresp)) {
			return 0;
		}
                if(j>200){console(F("overflow\n"),2); return -1;}
            }
        }
    }
    return -1;
}

int CircusESP01Lib::checkTCP() {
    char rOK[] = "OK\r\n";
    char sendOK[] = "SEND OK\r\n";
    char sdcommand[] = "AT+CIPSEND=1\r\n";
    if(sendcommand(sdcommand,rOK,1000L,1)==0) {
        char content[] = "W\r\n";
        return(sendcommand(content,sendOK,1000L,1));
    } else {
        return -1;
    }
}

int CircusESP01Lib::checkWIFI() {
    char gotIP[] = "STATUS:2\r\n";
    char disconnected[] = "STATUS:4\r\n";
    char statuscommand[] = "AT+CIPSTATUS\r\n";
    int r1 = sendcommand(statuscommand,gotIP,5000L,1);
    int r2 = sendcommand(statuscommand,disconnected,5000L,1);
    if(r1==0 || r2==0) {return 0;} else {return -1;}
}

int CircusESP01Lib::checkESP8266() {
    char rOK[] = "OK\r\n";
    char atcommand[] = "AT\r\n";
    return(sendcommand(atcommand,rOK,5000L,5));
}

int CircusESP01Lib::setESP() {
	char rOK[] = "OK\r\n";
	char ready[] = "ready\r\n";

        console(F("\n[CircusESP01Lib] Setting ESP8266 parameters\n"),1);

        // settings
        char releaseAPcommand[] = "AT+CWQAP\r\n";
        sendcommand(releaseAPcommand,rOK,1000L,1);
        char echocommand[] = "ATE0\r\n";
	if(sendcommand(echocommand,rOK,5000L,2) != 0) { // Modem echo off
		return -1;
	}
	char cwmcommand[] = "AT+CWMODE=1\r\n";
	if(sendcommand(cwmcommand,rOK,5000L,2) != 0) {
		return -1;
	}
	char muxcommand[] = "AT+CIPMUX=0\r\n";
	if(sendcommand(muxcommand,rOK,5000L,2) != 0) {
            return -1;
	}
	char infocommand[] = "AT+CIPDINFO=1\r\n";
	if(sendcommand(infocommand,rOK,5000L,5) != 0) {
		return -1;
	}
	char autocommand[] = "AT+CWAUTOCONN=0\r\n";
	if(sendcommand(autocommand,rOK,5000L,5) != 0) {
		return -1;
	}
	char dhcpcommand[] = "AT+CWDHCP=1,1\r\n";
	if(sendcommand(dhcpcommand,rOK,5000L,5) != 0) {
		return -1;
	}
        char rstcommand[] = "AT+RST\r\n";
	if(sendcommand(rstcommand,ready,5000L,5) != 0) {
		return -1;
	}

	return 0;
}

int CircusESP01Lib::connectWIFI() {
	char rOK[] = "OK\r\n";
        char command[50];

	console(F("\n[CircusESP01Lib] Connect to Wifi\n"),1);

        sprintf_P(command, PSTR("AT+CWJAP_CUR=\"%s\",\"%s\"\r\n"),_ssid,_pass);
	if(!sendcommand(command,rOK,10000L,1) == 0) { // Get connection
		return -1;
	}

        char conwcommand[] = "AT+CWJAP?\r\n";
	if(!sendcommand(conwcommand,rOK,2000L,1) == 0) { // Check connection
		return -1;
	}

        char cifcommand[] = "AT+CIFSR\r\n";
	if(!sendcommand(cifcommand,rOK,2000L,1) == 0) { // Check connection
		return -1;
	}

	return 0;
}

int CircusESP01Lib::connectServer() {

	char rOK[] = "OK";
        char already[] = "ALREADY CONNECTED";
	char mustclose[] = "must close ssl link";
	char error[] = "ERROR";

	char csizecommand[] = "AT+CIPSSLSIZE=4096\r\n";
	char closecommand[] = "AT+CIPCLOSE\r\n";

	console(F("\n[CircusESP01Lib] Stablishing Secure SSL connection\n"),1);

        if(!sendcommand(csizecommand,rOK,5000L,1) == 0) {
		return -1;
	}

        char startcommand[] = "AT+CIPSTART=\"SSL\",\"circusofthings.com\",443\r\n";
	if(sendcommand(startcommand,rOK,15000L,1) == 0) { // Get status
		return 0;
	} else {
		if(sendcommand(startcommand,already,2000L,1) == 0) {
			return 0;
		} else {
			return -1;
		}
	}


}

int CircusESP01Lib::mef(){
	while(checkESP8266()!=0)
		console(F("\n[CircusESP01Lib] Module not detected\n"),1);
	console(F("\n[CircusESP01Lib] Module detected\n"),1);
    	if(setESP()==0) {
		console(F("\n[CircusESP01Lib] Module set\n"),1);
		delay(500);
	} else {
		console(F("\n[CircusESP01Lib] Module set error\n"),1);
		return -1;
    	}
	if(connectWIFI()==0) {
    		console(F("\n[CircusESP01Lib] Wifi connected\n"),1);
    		delay(500);
	} else {
    		console(F("\n[CircusESP01Lib] Wifi connection error\n"),1);
    		return -2;
	}
	if(connectServer()>-1) {
		console(F("\n[CircusESP01Lib] Secure SSL connection stablished\n"),1);
		delay(500);
	} else {
		console(F("\n[CircusESP01Lib] Server connection failed\n"),1);
		return -2;
	}
	return 0;
}

void CircusESP01Lib::begin() {
	counter=0;
	esp01Serial->begin(_esp01BaudRate);
	while(mef()!=0)
		delay(1000);
}

int CircusESP01Lib::count(char *text) {
    	for(int i=0;i<300;i++)
        	if(text[i]=='\0')
            		return(i);
    	return -1;
}

char* CircusESP01Lib::parseServerResponse(char *r, char *label, int offset) {
    	int labelsize = count(label);
    	char *ini = strstr(r,label) + labelsize + offset;
    	static char content[100];
    	int i=0;
    	while (*ini!='\0'){ // may truncated occur
        	if(*ini!='\"')
            		content[i]=*ini;
        	else {
            		content[i]='\0';
            		return content;
        	}
        	i++; ini++;
    	}
    	return (char)0;
}




char* CircusESP01Lib::waitResponse(int timeout) {
	static char responsebody[100];
	for( int i = 0; i < sizeof(responsebody);  ++i )
        	responsebody[i] = (char)0;
	int j = 0;
	int pick = 0;
	unsigned long startTime = millis();
        while (millis()-startTime < timeout) {
	    	while (esp01Serial->available()) {
		        char c = esp01Serial->read();
		        if(_debug>1) {Serial.print(c);}
		        if (c=='{') {pick=1;}
		        if(pick){responsebody[j]=c;j++;}
		        if (c=='}') {responsebody[j]='\0';pick=0;Serial.print("\n");return responsebody;}
		}

	}
    	return responsebody;
}

void CircusESP01Lib::write(char *key, double value) {

    	counter=counter+1;
	console(F("Hits: "),2);
	console(counter,2);
	console("\n",2);

	char bufValue[15];
    	dtostrf(value,1,4,bufValue);
    	char requestLine[] = "PUT /WriteValue HTTP/1.1\r\n";
	char header2[] = "Host:www.circusofthings.com\r\n";
	char header3[] = "User-Agent:CircusESP01Lib-3.0.0\r\n";
	char header4[] = "Content-Type:application/json\r\n";
	char header5[] = "Content-Length: ";
    	char body[130];
	for( int i = 0; i < sizeof(body);  ++i )
        	body[i] = (char)0;
    	strcat(body, "{\"Key\":\""); strcat(body, key);
    	strcat(body, "\",\"Value\":"); strcat(body, bufValue);
    	strcat(body, ",\"Token\":\""); strcat(body, _token); strcat(body, "\"}");
	int countBody = count(body);
	char countBodyCharArray[16];
	itoa(countBody, countBodyCharArray, 10);
	strcat(header5, countBodyCharArray); strcat(header5, "\r\n");
    	int messageLength = count(requestLine) + count(header2) + count(header3) + count(header4) + count(header5) + 2 + count(body);

    	char listen[] = ">";
    	char cmdBuf[20];
    	sprintf_P(cmdBuf, PSTR("AT+CIPSEND=%d\r\n"), messageLength);

    	if(sendcommand(cmdBuf,listen,1000L,1)==0) {
		esp01Serial->print(requestLine);
		console(requestLine,2);
		esp01Serial->print(header2);
    		console(header2,2);
		esp01Serial->print(header3);
    		console(header3,2);
		esp01Serial->print(header4);
		console(header4,2);
		esp01Serial->print(header5);
		console(header5,2);
		esp01Serial->print("\r\n");
		console("\r\n",2);
		esp01Serial->print(body);
		console(body,2);
		console(F("\n"),2);
		char *responsebody = waitResponse(5000);
		console(responsebody,2);
		console(F("\n"),2);
		if((responsebody[0] == '{')){
			char labelk[] = "Key";
			char *key = parseServerResponse(responsebody, labelk,3);
			if(_debug==2) {console('\n',1);}
			console(F("[CircusOfThings.com] "),1);
			console(key,1);
			char labelm[] = "Message";
			char *message = parseServerResponse(responsebody, labelm,3);
			console(F(" - "),1);
			console(message,1);
			console(F("\n"),1);
			return;
		} else {
			console(F("[CircusESP01Lib] Unknown response\n"),1);
			mef();
			return -1;
		}
	} else {
		console(F("[CircusESP01Lib] Can't send READ command\n"),1);
	    	mef();
		return -1;
	}
}

double CircusESP01Lib::read(char *key) {

    	counter=counter+1;
	console(F("Hits: "),2);
	console(counter,2);
	console("\n",2);

	char requestLine[250];
    	sprintf_P(requestLine, PSTR("GET /ReadValue?Key=%s&Token=%s HTTP/1.1\r\n"), key, _token);
	char header2[] = "Host:www.circusofthings.com\r\n";
	char header3[] = "User-Agent:CircusESP01Lib-3.0.0\r\n";
	char header4[] = "Content-Type:application/json\r\n";
	int messageLength = count(requestLine) + count(header2) + count(header3) + count(header4) + 2;

	char listen[] = ">";
    	char cmdBuf[20];
    	sprintf_P(cmdBuf, PSTR("AT+CIPSEND=%d\r\n"), messageLength);

    	if(sendcommand(cmdBuf,listen,5000L,1)==0) {
		esp01Serial->print(requestLine);
		console(requestLine,2);
		esp01Serial->print(header2);
		console(header2,2);
        	esp01Serial->print(header3);
		console(header3,2);
        	esp01Serial->print(header4);
		console(header4,2);
		esp01Serial->print("\r\n");
		console("\r\n",2);

		char *responsebody = waitResponse(5000);
		if (responsebody!=(char)0) {
			char labelk[] = "Key";
			char *key = parseServerResponse(responsebody, labelk, 3);
			if(_debug==2) {console('\n',1);}
			console(F("[CircusOfThings.com] "),1);
			console(key,1);
			char labelm[] = "Message";
			char *message = parseServerResponse(responsebody, labelm, 3);
			console(F(" - "),1);
			console(message,1);
			char labelv[] = "\"Value";
			char *value = parseServerResponse(responsebody, labelv, 2);
			console(F(" - "),1);
			console(value,1);
			console('\n',1);
			return(atof(value));
		} else {
			console(F("[CircusESP01Lib] Unknown response\n"),1);
			mef();
			return -1;
		}
	} else {
		console(F("[CircusESP01Lib] Can't send READ command\n"),1);
	    	mef();
		return -1;
	}

}
