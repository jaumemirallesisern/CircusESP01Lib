/*

  This example code shows how to read a signal using the circusofthings.com API through its CircusESP01Lib-1.1.0 library for Arduino.

  This code will read a set point value at Circus. Then make this value to be the sped for the local fan.

  A software serial port is used, so the onboard serial port is used to monitor the process. You have 3 degrees for monitor: DEBUG_NO,DEBUG_YES and DEBUG_DEEP.

  There are no 3rd part libraries to use, beside SoftwareSerial.

  Created by Jaume Miralles Isern, Dec 16, 2021.
*/


#include <CircusESP01Lib.h>

// ------------------------------------------------
// These are the CircusESP01Lib related declarations
// ------------------------------------------------

char ssid[] = "your_SSID_here";             // Place your wifi SSID here
char password[] =  "your_password_here";    // Place your wifi password here
char token[] = "your_token_here";           // Place your token, find it in 'account' at Circus. It will identify you.
char server[] = "www.circusofthings.com";
char fanSpeed_key[] = "5115";             // The key that will give the set point from circusofthings.com
int TXPinToESP01 = 2;                     // IO port in your arduino that you will use as TX for serial communication with the EP01 module
int RXPinFromESP01 = 3;                   // IO port in your arduino that you will use as RX for serial communication with the EP01 module
SoftwareSerial ss(RXPinFromESP01,TXPinToESP01);
int esp01BaudRate = 9600;                 // This rate has to match with the one set in ESP01. Very high rates here tend to be problematic.
int debugLevel = DEBUG_YES;               // Change it your will. DEBUG_NO, DEBUG_YES or DEBUG_DEEP
int enableSSL = 0;                        // Use SSL encryption or not when comunicating with CoT. 0 -> Dsiable. 1 -> Enable.
CircusESP01Lib circusESP01(&ss,esp01BaudRate,server,token,ssid,password,debugLevel,enableSSL);



// ------------------------------------------------
// These are the Fan Speed Example related declarations
// ------------------------------------------------

// Fan control pins connected to driver
int FanIN3 = 9; // pin 9 will be the Nano output connected to the IN3 driver input
int FanIN4 = 10; // pin 10 will be the Nano output connected to the IN4 driver input
int FanENB = 11; // pin 11 will be the Nano output, providing PWM, connected to the ENB driver input
// PWM properties in Nano:
// resolution is 8 bit, then the range for duty cicle is from 0 (0%) to 255 (100%)
int dutyCycle = 0; // let's start with the fan stopped

void setup() {
    Serial.begin(115200); // Remember to match this value with the baud rate in your console
    circusESP01.begin();  // start Circus library for a seamless code without dealing with AT commands and ESP01

    // set pins as outputs
    pinMode(FanIN3, OUTPUT);
    pinMode(FanIN4, OUTPUT);
    pinMode(FanENB, OUTPUT);
    digitalWrite(FanIN4, LOW);
    digitalWrite(FanIN3, HIGH);
}

void loop() {
    // This will read the value of my signal at Circus. I'm supposed to set it between 0 and 255.
    float fanSpeed_setPoint = circusESP01.read(fanSpeed_key);
    // As I defined at Circus the range (0-255) as the duty cycle of the Nano (0-255), there's no need to make aditional conversions.
    // If you want to set different ranges, use the line code below to make the proper linear conversion
    dutyCycle = fanSpeed_setPoint;
    // Set a new PWM duty cycle in output, thus a new fan speed
    analogWrite(FanENB,dutyCycle);
    // Some debug in console just to check
    Serial.print("Fan duty cycle (0-255) set to: ");
    Serial.println(dutyCycle);
    delay(3000);

}
