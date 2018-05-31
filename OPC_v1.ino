/*
Joseph Habeck, May 2018

Version 1: STAND-ALONE MODE

Summary: code to run the OPC-N2 in STAND-ALONE MODE. This code builds off of HeaterV2.ino
         but adds another relay switch to turn on the OPC.  

The circuit includes:
-- Arduino Uno
-- Sparkfun micro SD shield (chip select pin = 8)
-- OMRON G5V-2 relay switch (digital pin 4)
-- TMP analog temperature sensor (analog pin 0)
-- Dallas digital temperature sensor (digital pin 2)
-- (2) 9v battery power supply 
-- GoPro battery pack
-- Alphasense OPC-N2 
-- TE QUAZ-SH-105D relay switch (digital pin 9)
 */


#include <SPI.h>
#include <SD.h>

// Used for Dallas temp sensor:
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
//
int x = 1;

// Set pins
int tPin1 = 0;                    // set analog pin 0 (analog temp sensor)
int tPin2 = 2;                    // set digital pin 2 (digital temp sensor)
//int tPin3 = 5;                  // set analog pin 5 (analog temp sensor)
int qSwitch = 4;                  // set digital pin 4 (heater relay)
int opcSwitch = 9;                // set digiral pin 9 (OPC relay)
    
// Define variables
float t1;                         // battery temp (analog) 
float t2;                         // OPC temp (digital)
//float t3;                       // outside temp (digital)
float samp_freq=1400;             // sampling frequency (ms)
float t_low = 283;                // critical low temp (heater turns ON if t2 < t_low)
float t_high = 289;               // critical high temp (heater turns OFF if t2 > t_high)
int hold;                         // used for heater switch

// More variables
String data;                      // used for data logging
String heaterStatus;              // used for data logging; will be "on" or "off" 
String filename = "tempLog.csv";  // file name that data wil be written to
const int CS = 8;                 // CS pin on SD card
File tempLog;                     // file that data is written to 
unsigned long FT;                 // flight-time

// Setup
void setup() {
  Serial.begin(9600);             // open serial port
  sensors.begin();                // start up Dallas temp sensor library 
  
  // Initalize digital pins:
  pinMode(tPin2, INPUT);
  pinMode(qSwitch, OUTPUT);
  pinMode(opcSwitch, OUTPUT);
  
  // Open serial port(s):
  while (!Serial){
    ; // wait for serial port to connect
  }

  Serial.print("Initializing SD card...");

  // Check if card is present/initalized: 
  if (!SD.begin(CS)){
  Serial.println("card initialization FAILED - something is wrong..."); // card not present or initialization failed
  while (1); // dont do anything more
  }
  
  Serial.println("card initialization PASSED... bon voyage!"); // initialization successful

  // Initialize file:
  tempLog = SD.open(filename, FILE_WRITE); // open file
  
  if (tempLog) {
    Serial.println( filename + " opened...");
    tempLog.println("Temperature 1 (battery) (K), Temperature 2 (OPC) (K), Temperature 3 (atmos.) (K), Heater Status, Flight Time (hour:min:sec)"); // file heading
    tempLog.close();
    Serial.println("File initialized... begin data logging!");
  }
  else {
    Serial.println("error opening file");
    return;
  }
  digitalWrite(opcSwitch, HIGH);  // power on OPC
  Serial.println("OPC qued... please wait ~60 seconds"); // (OPC takes approx. 60 sec to turn on in Standalone mode)
}


void loop() {
  FT = millis() / 1000; // set flight-time variable to Arduino internal clock
  
  ////////// Temperature monitoring ////////// 
  
  t1 = getAnalogTemp(tPin1);              // analog temp in Kelvin
  t2 = sensors.getTempCByIndex(0);        // digital temp in celcius
  t2 = t2 + 273.15;                       // digital temp in Kelvin
  t3 = getAnalogTemp(tPin3);              // digital temp in celcius
  t3 = t3 + 273.15;

  ////////// Heater operation //////////

// "test-fire" heater after Arduino clock has started; NOTE heater does not depend on temperature values during this time!
  if (FT<(60)){
   digitalWrite(qSwitch, HIGH); 
   heaterStatus = "on";
   Serial.println("testing heater");
  }
  
// compare digital temp. to critical temp.:  
  else{
    if (t1 < t_low) {
      hold = 1; // if temperature is below low critical temperature
    }
    if (t1 > t_high) {
      hold = 0; // if temperature is above high critical temperature
     }    

// turn heater on/off:
    if (hold==1){
    digitalWrite(qSwitch, HIGH); 
    heaterStatus = "on";
    }
   else {
    digitalWrite(qSwitch, LOW);
    heaterStatus = "off";
    }  
  }

////////// Datalogging //////////

data = "";
data += t1;               
data += ",";               
data += t2;
data += ",";
data += t3;
data += ",";
data += heaterStatus;     // log heater status (either "on" or "off")
data += ",";
data += flightTime(FT);    // log flight time; flightTime is a user-defined function

////////// Data Writing //////////

 tempLog = SD.open("tempLog.csv", FILE_WRITE); // open file

 if (tempLog) {
    //Serial.println("tempLog.csv opened..."); // file open successfully 
    tempLog.println(data);
    tempLog.close();
    Serial.println(data);
  }
  else {
    Serial.println("error opening file"); // file open failed
    return;
  }

 delay(samp_freq); // delay 1 second i.e. do all that every 1 second 
x=x+1;
 }

///////// User-defined functions //////////

// Reads in temp. from analog sensor and converts it to Kelvin; written by: Joey Habeck
float getAnalogTemp(int pin) {
  float t = analogRead(pin) * .004882814; // analog temperature
  float t_F = (((t - .5) * 100) * 1.8) + 32; // convert temperature to Farenheit
  float t_K = (t_F + 459.67) * 5 / 9; // convert Farenheit to Kelvin
  return (t_K);
}

// Reads in time from Arduino clock (seconds) and converts it to hr:min:sec; written by: Simon Peterson 
String flightTime(unsigned long t) {
  String fTime = "";
  fTime += (String(t / 3600) + ":");
  t %= 3600;
  fTime += String(t / 600);
  t %= 600;
  fTime += (String(t / 60) + ":");
  t %= 60;
  fTime += (String(t / 10) + String(t % 10));
  return fTime;
}

