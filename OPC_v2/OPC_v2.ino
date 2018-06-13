

/*
Joseph Habeck, May 2018

Last edited:
  Jack Stutler 6/5/2018
    - Added two more Dallas digital temperature sensors and 1 more heater pad

Version 1: STAND-ALONE MODE

Summary: code to run the OPC-N2 in STAND-ALONE MODE. This code builds off of HeaterV2.ino
         but adds another relay switch to turn on the OPC. Includes digital temperature sensors
         to monitor OPC, battery and atmospheric temperatures. Monitors OPC and battery
         temperatures and turns on heating pads for the OPC and battery if needed independently.  

The circuit includes:
-- Arduino Uno
-- Sparkfun micro SD shield (chip select pin = 8)
-- TE OUAZ-SH-105D relay switch (digital pin 2)
-- TE OUAZ-SH-105D relay switch (digital pin 6)
-- Dallas digital temperature sensor (digital pin 3)
-- Dallas digital temperature sensor (digital pin 4)
-- Dallas digital temperature sensor (digital pin 5)
-- (2) 9v battery power supply 
-- GoPro battery pack
-- Alphasense OPC-N2 
 */


#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>

// Used for Dallas temp sensor:
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 3
#define TWO_WIRE_BUS 30
#define THREE_WIRE_BUS 5
#define FOUR_WIRE_BUS 7
#define chipSelect 4 
#define dataLED 27
#define fixLED 28
#define ledSD 29
#define qSwitchOPC 2
#define qSwitchBatt 6
#define opcSwitch 9

OneWire oneWireOne(ONE_WIRE_BUS);
OneWire oneWireTwo(TWO_WIRE_BUS);
OneWire oneWireThree(THREE_WIRE_BUS);
OneWire oneWireFour(FOUR_WIRE_BUS);

DallasTemperature sensors1(&oneWireOne);        // Temp sensor for OPC
DallasTemperature sensors2(&oneWireTwo);        // Temp sensor for Battery
DallasTemperature sensors3(&oneWireThree);      // Temp sensor for Atmosphere
DallasTemperature sensors4(&oneWireFour);      // Temp sensor for ambient box temperature

/////Sensor Suite////////
#include <TinyGPS++.h>
#include <SparkFun_ADXL345.h>

TinyGPSPlus GPS;
int GPSstartTime;
boolean newDay = false;
boolean firstFix = false;
int days = 0;
boolean newData = false;
int checkTime;
String sensorSuite = "";

ADXL345 adxl = ADXL345();
int x,y,z;

// Set pins
//int tPin1 = 3;                    // set digital pin 3 (digital temp sensor) OPC
//int tPin2 = 4;                    // set digital pin 4 (digital temp sensor) BATT
//int tPin3 = 5;                    // set digital pin 5 (digital temp sensor) Atmos.
//int tpin4 = 7;
//int qSwitchOPC = 2;               // set digital pin 2 (OPC heater relay 1)
//int qSwitchBatt = 6;              // set digital pin 6 (Battery relay 2)
//int opcSwitch = 9;                // set digiral pin 9 (OPC relay)
    
// Define variables
float t1;                         // OPC temp (digital) 
float t2;                         // Battery temp (digital)
float t3;                         // outside temp (digital)
float t4;
float samp_freq=5000;             // sampling frequency (ms)
long lastTime = 0;

float t_low = 283;                // critical low temp (heater turns ON if t1,t2 < t_low)
float t_high = 289;               // critical high temp (heater turns OFF if t1,t2 > t_high)
int hold1;                        // used for OPC heater switch
int hold2;                        // used for Battery heater switch

// More variables
String data;                      // used for data logging
String heaterStatusOPC;           // used for data logging; will be "OPC ON" or "OPC OFF" 
String heaterStatusBatt;          // used for data logging; will be "Batt ON" or "Batt OFF"
String filename = "";  // file name that data wil be written to
boolean flightLogOpen = false;
boolean OPC = false;

unsigned long FT = 0;
File fLog;
String testName = "";
boolean testFileOpen = false;
String test = "";

//////////////////LED Stuff////////
class action {
  protected:
    unsigned long Time;
    String nam;
  public:
    String getName();
};
class Blink: public action {
  protected:
    int ondelay;
    int offdelay;
    int ontimes;
  public:
    friend void blinkMode();
    void BLINK();
    Blink();
    Blink(int on, int off, int times, String NAM, unsigned long tim);
    int getOnTimes();
};
Blink recoveryBlink = Blink(200, 2000, -1, "recoveryBlink", 0);
Blink countdownBlink = Blink(200, 850, -1, "countdownBlink", 0);
Blink* currentBlink = &countdownBlink;

boolean LEDon = false;
#define FixDelay 1000
#define noFixDelay 15000
boolean recovery = false;


// Setup
void setup() {
  Serial.begin(9600);             // open serial port

  // Initialize sensor objects
  sensors1.begin();               
  sensors2.begin();
  sensors3.begin();
  sensors4.begin();

  // Initialize GPS Serial
  Serial1.begin(4800);

  // Initialize Accelerometer
  adxl.powerOn();
  adxl.setRangeSetting(16);
  adxl.setSpiBit(0);
  
  // Initalize digital pins:
  /*pinMode(tPin1, INPUT);  // OPC
  pinMode(tPin2, INPUT);  // Battery
  pinMode(tPin3, INPUT);  // Outside world
  pinMode(tpin4, INPUT);  // Inside box
  */
  pinMode(qSwitchOPC, OUTPUT);
  pinMode(qSwitchBatt, OUTPUT);
  pinMode(opcSwitch, OUTPUT);
  pinMode(chipSelect, OUTPUT);
  pinMode(dataLED, OUTPUT);
  pinMode(fixLED, OUTPUT);
  pinMode(ledSD, OUTPUT);
  //pinMode(53, OUTPUT);
  //digitalWrite(53, HIGH);
  
  //pinMode(53, OUTPUT);
  
  // Open serial port(s):
  while (!Serial){
    ; // wait for serial port to connect
  }

  Serial.print("Initializing SD card...");

  // Check if card is present/initalized: 
  if (!SD.begin(chipSelect)){
    Serial.println("card initialization FAILED - something is wrong..."); // card not present or initialization failed
    while (1){
    }    // dont do anything more
  }
  
  Serial.println("card initialization PASSED... bon voyage!"); // initialization successful
  for (int i = 0; i < 100; i++) {
    if (!SD.exists("flight" + String(i / 10) + String(i % 10) + ".csv")) {
      filename = "flight" + String(i / 10) + String(i % 10) + ".csv";
      fLog = SD.open(filename, FILE_WRITE); // open file
      //Serial.println(filename);
      flightLogOpen = true;
      digitalWrite(ledSD, HIGH);
      break;
    }
  }
  if (flightLogOpen) {
    Serial.println( filename + " opened...");
    fLog.println("Flight Time, Lat, Long, Altitude (ft), Date, Hour:Min:Sec, Fix, Accel x, Accel y, Accel z, Temperature (Box), Temperature 1 (OPC) (K), Temperature 2 (Battery) (K), Temperature 3 (atmos.) (K), OPC Heater Status, Batt. Heater Status, Flight Time (hour:min:sec)"); // file heading
    fLog.close();
    flightLogOpen = false;
    digitalWrite(ledSD, LOW);
    Serial.println("File initialized... begin data logging!");
  }
  else {
    Serial.println("error opening file");
    return;
  }
  
  // Initialize file:
  
  
  
 
  digitalWrite(opcSwitch, HIGH);  // power on OPC
  Serial.println("OPC qued... please wait ~60 seconds"); // (OPC takes approx. 60 sec to turn on in Standalone mode)
}


void loop() {
  //FT = millis() / 1000; // set flight-time variable to Arduino internal clock
  blinkMode();
  Fixblink();
  
  // Request temperatures for all sensors
  sensors1.requestTemperatures();
  sensors2.requestTemperatures();
  sensors3.requestTemperatures();
  sensors4.requestTemperatures();
  
  ////////// Temperature monitoring ////////// 
  
  t1 = sensors1.getTempCByIndex(0);        // OPC digital temp in celcius
  t1 = t1 + 273.15;
  t2 = sensors2.getTempCByIndex(0);        // Battery digital temp in celcius
  t2 = t2 + 273.15;                       
  t3 = sensors3.getTempCByIndex(0);        // Outside world digital temp in celcius
  t3 = t3 + 273.15;
  t4 = sensors4.getTempCByIndex(0);
  t4 = t3 + 273.15;

  ////////// Heater operation //////////

// "test-fire" heater after Arduino clock has started; NOTE heater does not depend on temperature values during this time!
  if ((millis()/1000)<(60)){
   digitalWrite(qSwitchOPC, HIGH); 
   heaterStatusOPC = "OPC ON";
   //Serial.println("testing OPC heater");
   
   digitalWrite(qSwitchBatt, HIGH);
   heaterStatusBatt = "Batt ON";
   //Serial.println("testing BATT heater");
  }
  
// compare digital temp. to critical temp.:  
  else{
    // Check OPC temp
    if (t1 < t_low){
      hold1 = 1; // if OPC temperature is below low critical temperature
    }
    if (t1 > t_high){
      hold1 = 0; // if OPC temperature is above high critical temperature
    }
    // Check Battery temp
    if (t2 < t_low){
      hold2 = 1; // if Batt temperature is below low critical temperature
    }
    if (t2 > t_high){
      hold2 = 0; // if Batt temperature is above high critical temperature
    }

// turn heater on/off:
    // OPC heater
    if (hold1==1){
      digitalWrite(qSwitchOPC, HIGH); 
      heaterStatusOPC = "OPC ON";
    }
    else{
      digitalWrite(qSwitchOPC, LOW);
      heaterStatusOPC = "OPC OFF";
    }
    // Battery heater
    if (hold2==1){
      digitalWrite(qSwitchBatt, HIGH);
      heaterStatusBatt = "BATT ON";
    }
    else{
      digitalWrite(qSwitchBatt, LOW);
      heaterStatusBatt = "BATT OFF";
    }
  }

////////// Datalogging //////////
updateGPS();
data = "";
data = sensorSuite;
data += ",";
data += t1;               
data += ",";               
data += t2;
data += ",";
data += t3;
data += ",";
data += heaterStatusOPC;     // log OPC heater status (either "OPC ON" or "OPC OFF")
data += ",";
data += heaterStatusBatt;    // log battery heater status (either "Batt ON" or "Batt OFF")
data += ",";
data += flightTimeStr();      // log flight time; flightTime is a user-defined function
test="1,2,3,4";

////////// Data Writing //////////
if(millis()-lastTime>=samp_freq){
 fLog = SD.open(filename, FILE_WRITE); // open file
 flightLogOpen = true;
 digitalWrite(ledSD, HIGH);
 if (flightLogOpen) {
    //Serial.println("flightLog.csv opened..."); // file open successfully 
    fLog.println(data);
    Serial.println(data);
    fLog.close();
    flightLogOpen = false;
    digitalWrite(ledSD, LOW);
    
  }
  else {
    Serial.println("error opening file"); // file open failed
    return;
  }
  
  lastTime=millis();
}

  //delay(samp_freq); // delay 1 second i.e. do all that every 1 second 

} // End main loop


///////// User-defined functions //////////

// Reads in temp. from analog sensor and converts it to Kelvin; written by: Joey Habeck
float getAnalogTemp(int pin) {
  float t = analogRead(pin) * .004882814; // analog temperature
  float t_F = (((t - .5) * 100) * 1.8) + 32; // convert temperature to Farenheit
  float t_K = (t_F + 459.67) * 5 / 9; // convert Farenheit to Kelvin
  return (t_K);
}

// Reads in time from Arduino clock (seconds) and converts it to hr:min:sec; written by: Simon Peterson 
/*String flightTime(unsigned long t) {
  String fTime = "";
  fTime += (String(t / 3600) + ":");
  t %= 3600;
  fTime += String(t / 600);
  t %= 600;
  fTime += (String(t / 60) + ":");
  t %= 60;
  fTime += (String(t / 10) + String(t % 10));
  return fTime;
}*/
