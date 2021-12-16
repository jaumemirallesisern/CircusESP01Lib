/*
  This example code that shows how to write a signal using the circusofthings.com API through its CircusESP01Lib-1.1.0 library for Arduino.

  This code will read the local status of a switch (Window open / close). Then will publish this value at Circus.

  A software serial port is used, so the onboard serial port is used to monitor the process. You have 3 degrees for monitor: DEBUG_NO,DEBUG_YES and DEBUG_DEEP.

  There are no 3rd part libraries to use, beside SoftwareSerial.

  Created by Jaume Miralles Isern, Dec 16, 2021.
*/


#include <CircusESP01Lib.h>

// ------------------------------------------------
// These are the CircusESP01Lib related declarations
// ------------------------------------------------

char ssid[] = "your_SSID_here";             // Place your wifi SSID here
char password[] =  "your_password_here";        // Place your wifi password here
char token[] = "your_token_here";           // Place your token, find it in 'account' at Circus. It will identify you.
char server[] = "www.circusofthings.com";
char windowSignalKey[] = "16035";         // The key of the signal you that exists at circusofthings.com
int TXPinToESP01 = 2;                     // IO port in your arduino that you will use as TX for serial communication with the EP01 module
int RXPinFromESP01 = 3;                   // IO port in your arduino that you will use as RX for serial communication with the EP01 module
SoftwareSerial ss(RXPinFromESP01,TXPinToESP01);
int esp01BaudRate = 9600;                 // This rate has to match with the one set in ESP01. Very high rates here tend to be problematic.
int debugLevel = DEBUG_YES;               // Change it your will. DEBUG_NO, DEBUG_YES or DEBUG_DEEP
int enableSSL = 0;                        // Use SSL encryption or not when comunicating with CoT. 0 -> Dsiable. 1 -> Enable.
CircusESP01Lib circusESP01(&ss,esp01BaudRate,server,token,ssid,password,debugLevel,enableSSL);



// ------------------------------------------------
// These are the Window experiment related declarations
// ------------------------------------------------

int windowPin = 8;




void setup() {
    Serial.begin(115200);       // Remember to match this value with the baud rate in your console
    circusESP01.begin();        // start Circus library for a seamless code without dealing with AT commands and ESP01
    pinMode(windowPin, INPUT);  // set pin  for the window switch as an input
}

void loop() {
    // Get the actual state of your window.
    // When door open, switch opens, and a "1" is presented to the nano pin due to the (external) Pull-up resistor in our scheme
    // When door close, switch closes, and a "0" is presented to the nano pin because switch contacts GND
    double val = digitalRead(windowPin);
    // Show on console
    Serial.print("Window value is: ");Serial.println(val);
    // Report this state to the Circus
    circusESP01.write(windowSignalKey,val);
    delay(10000);
}
