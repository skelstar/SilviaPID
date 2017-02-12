#include <myWifiHelper.h>
#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <EventManager.h>
#include <TaskScheduler.h>
#include <myPushButton.h>
#include "appconfig.h"

#define WATER_LOW   0
#define COFFEE_SW   1
#define FUEL_BAR    2

/* ----------------------------------------------------------- */

char versionText[] = "SilvaPID v1.2.0";

#define PIXEL_PIN       15
#define FLOODLIGHT_PIN  2
#define NUMPIXELS       8          // pixel stick
#define NUMFLOODPIXELS  4*4
#define DEBUG_PIN       12
#define BREW_PIN        13
#define WATER_PIN       14

#define PULL_UP     true

/* ----------------------------------------------------------- */

Adafruit_NeoPixel statusStick = Adafruit_NeoPixel(NUMPIXELS, PIXEL_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel floodLight = Adafruit_NeoPixel(NUMFLOODPIXELS, FLOODLIGHT_PIN, NEO_GRB + NEO_KHZ800);

#define STATUS_FLASHING     10

uint32_t STATUS_COLOUR_OFF = statusStick.Color(0, 0, 0);
uint32_t STATUS_ERROR_COLOUR = statusStick.Color(255, 0, 0);
uint32_t STATUS_UNKNOWN_COLOUR = statusStick.Color(0, 0, 0);
uint32_t FLOOD_RED_COLOR = floodLight.Color(255, 0, 0);

/* ----------------------------------------------------------- */

struct Channel {
        int index;
        uint32_t color;
        uint32_t colorOff;
        int eventCode;
        int state;
        int prevState;
        int onState;        // either 1, or 0
        int debugPin;
        int i2cAddr;
};

#define NUM_CHANNELS    3
Channel channel[NUM_CHANNELS] = {	
	{
	    WATER_LOW,                      // index
        statusStick.Color(0, 0, 50),    // color
        statusStick.Color(0, 0, 1),     // colorOff
        EventManager::kEventUser0,      // eventCode
        -2,                             // state                             
        -2,                             // prevState
        0,                              // onState       
        WATER_PIN,
        WATER_I2C
    },
    {
    	COFFEE_SW,                   
        statusStick.Color(50, 15, 0),
        statusStick.Color(2, 1, 0),  
        EventManager::kEventUser2,   
        -2,
        -2,
        1,
        BREW_PIN,
        COFFEE_SW_I2C
    },
    {		
		FUEL_BAR,                     
        statusStick.Color(0, 0, 0),   
        statusStick.Color(0, 0, 0),   
        EventManager::kEventUser3,    
        -2,
        -2,
        1,
        NULL,
        NULL
    }
};

/* ----------------------------------------------------------- */

long ticks = 0;

int counter = 1;
bool ledState = false;
long lastToggled = millis();
long fuelBarTick = 0;

EventManager gEM;

void serviceEvent(int st);
int getI2cChannelState();
void setChannelPixel(Channel channel, int val);
void setBlockChunkToColor(int index, int val);
void eventcallback_WaterLow( int event, int waterlevel );
void listener_Flashing(int event, int statusBit);

myPushButton debugPin(DEBUG_PIN, PULL_UP, 100000, HIGH, NULL);
myPushButton brewPin(channel[COFFEE_SW].debugPin, PULL_UP, 100000, HIGH, NULL);
myPushButton waterPin(channel[WATER_LOW].debugPin, PULL_UP, 100000, HIGH, NULL);

int lastDebugVal = 0;
long lastUpdate;

bool debugMode = false;

MyWifiHelper wifiHelper(WIFI_HOSTNAME);


/* ----------------------------------------------------------- */

void tFlashWaterONCallback();
void tFlashWaterOFFCallback();

Scheduler runner;

Task tFlashWater(800, TASK_FOREVER, &tFlashWaterONCallback, &runner, true);

void tFlashWaterONCallback() {
    statusStick.setPixelColor(channel[WATER_LOW].index, channel[WATER_LOW].color);
    statusStick.show();
    tFlashWater.setCallback(&tFlashWaterOFFCallback);
}

void tFlashWaterOFFCallback() {
    statusStick.setPixelColor(channel[WATER_LOW].index, statusStick.Color(0,0,0));
    statusStick.show();
    tFlashWater.setCallback(&tFlashWaterONCallback);
}

/* ----------------------------------------------------------- */
void setup() {
    
    Serial.begin(9600);
    Serial.println("Booting");
    Serial.println(versionText);

    statusStick.begin();
    statusStick.setPixelColor(0, statusStick.Color(0, 255, 0));
    statusStick.show();

    floodLight.begin();
    for (int i=0; i<NUMFLOODPIXELS; i++) {
        floodLight.setPixelColor(i, FLOOD_RED_COLOR);
    }
    floodLight.show();

    Wire.begin();

    // debug mode?
    debugMode = inDebugMode();
    if (debugMode) {
    	Serial.println("DEBUG MODE");
    }

    gEM.addListener(channel[WATER_LOW].eventCode, eventcallback_WaterLow);
    gEM.addListener(channel[COFFEE_SW].eventCode, eventcallback_CoffeeSw);
    
    delay(1000);

    runner.addTask(tFlashWater);
    
    // get init states
    channel[WATER_LOW].state = getI2cChannelState(channel[WATER_LOW]);
    eventcallback_WaterLow(channel[WATER_LOW].eventCode, channel[WATER_LOW].state);

    channel[COFFEE_SW].state = getI2cChannelState(channel[COFFEE_SW]);
    eventcallback_CoffeeSw(channel[COFFEE_SW].eventCode, channel[COFFEE_SW].state);

    wifiHelper.setupWifi();
    wifiHelper.setupOTA(WIFI_OTA_NAME);
}

/* ----------------------------------------------------------- */

void loop() {
    
    ArduinoOTA.handle();

    gEM.processEvent();

    debugPin.serviceEvents();
    brewPin.serviceEvents();
    waterPin.serviceEvents();

    serviceWaterLevel();
    serviceBrewSwitch();
    //serviceFuelBarTick();

    runner.execute();

    delay(100);
}

/*-------------------------------------------------------*/

void serviceWaterLevel() {

    Channel waterCh = channel[WATER_LOW];
    int waterState = getI2cChannelState(waterCh);
    if (stateChanged(waterCh, waterState)) {
        channel[WATER_LOW].prevState = channel[WATER_LOW].state;
        channel[WATER_LOW].state = waterState;
        // Serial.println("Water state changed");
        gEM.queueEvent(waterCh.eventCode, waterState);
    }
}

void serviceBrewSwitch() {
            
    Channel brewCh = channel[COFFEE_SW];
    int brewState = getI2cChannelState(brewCh);

    if (stateChanged(brewCh, brewState)) {
        channel[COFFEE_SW].prevState = channel[COFFEE_SW].state;
        channel[COFFEE_SW].state = brewState;
        // Serial.println("Brew state changed");
        gEM.queueEvent(brewCh.eventCode, brewState);

        if (brewCh.state != brewCh.onState) {
            // switched off
            gEM.removeListener(channel[FUEL_BAR].eventCode, listener_FuelBar);
            initFuelBar();
        } else {
            fuelBarTick = millis();
            gEM.addListener(channel[FUEL_BAR].eventCode, listener_FuelBar);
        }
    }
}

/*-------------------------------------------------------*/

int getI2cChannelState(Channel ch) {

    if (debugMode) {
        if (ch.index == COFFEE_SW) {
            return brewPin.isPressed() ? 1 : 0;
        } else if (ch.index == WATER_LOW) {
            return waterPin.isPressed() ? 1 : 0;
        }
    }

    Wire.requestFrom(ch.i2cAddr, 1);

    while (Wire.available()) {
        byte c = Wire.read();
        
        if (c == 1 || c == 0) {
            return c;
        } else {
            return I2C_UNKNOWN_RESPONSE;
        }
    }
    return I2C_NO_RESPONSE;    
}

void updateFuelBar(int seconds) {
}

void initFuelBar() {
}

void setChannelPixel(Channel ch) {
    uint32_t color;

    int val = ch.state;
    Serial.print("setChannelPixel - index: "); Serial.print(ch.index); Serial.print(", val: "); Serial.println(val);

    if (val == I2C_NO_RESPONSE) {
        color = STATUS_ERROR_COLOUR;
    } else if (val == I2C_UNKNOWN_RESPONSE) {
        color = STATUS_UNKNOWN_COLOUR;
    } else if (val == ch.onState) {
        color = ch.color;
    } else {
        color = STATUS_COLOUR_OFF;
    }
    statusStick.setPixelColor(ch.index, color);
    statusStick.show();
}

/* ----------------------------------------------------------- */
void eventcallback_WaterLow( int event, int state ) {
    if (channel[WATER_LOW].state == channel[WATER_LOW].onState) {
        Serial.println("Restarting tFlashWater");
        tFlashWater.restart();
    } else {
        Serial.println("Disablng tFlashWater");
        tFlashWater.disable();
    }
    setChannelPixel(channel[WATER_LOW]);
}

void eventcallback_CoffeeSw( int event, int state ) {
    Serial.println("eventcallback_CoffeeSw");
    setChannelPixel(channel[COFFEE_SW]);
}

void listener_FuelBar(int event, int state) {
}

bool inDebugMode() {
    return debugPin.isPressed();
}

bool stateChanged(Channel ch, int newState) {
    return ch.state != newState;
}

void updatedState(Channel ch, int newState) {
    ch.prevState = ch.state;
    ch.state = newState;
}
