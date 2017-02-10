#include <myWifiHelper.h>
#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <EventManager.h>
#include <myPushButton.h>
#include "appconfig.h"

#define WATER_LOW   0
#define COFFEE_SW   1
#define FUEL_BAR   2

/* ----------------------------------------------------------- */

char versionText[] = "SilvaPID v1.2.0";

#define PIXEL_PIN       15
#define NUMPIXELS       8          // pixel stick
#define DEBUG_PIN       12
#define BREW_PIN        13
#define WATER_PIN       14

#define PULL_UP     true

/* ----------------------------------------------------------- */

Adafruit_NeoPixel statusStick = Adafruit_NeoPixel(NUMPIXELS, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

#define STATUS_FLASHING     10

uint32_t STATUS_COLOUR_OFF = statusStick.Color(0, 0, 0);
uint32_t STATUS_ERROR_COLOUR = statusStick.Color(255, 0, 0);
uint32_t STATUS_UNKNOWN_COLOUR = statusStick.Color(0, 0, 0);

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
void listener_WaterLevel( int event, int waterlevel );
void listener_Flashing(int event, int statusBit);

myPushButton debugPin(DEBUG_PIN, PULL_UP, 100000, HIGH, NULL);
myPushButton brewPin(channel[COFFEE_SW].debugPin, PULL_UP, 100000, HIGH, NULL);
myPushButton waterPin(channel[WATER_LOW].debugPin, PULL_UP, 100000, HIGH, NULL);

int lastDebugVal = 0;
long lastUpdate;

bool debugMode = false;

MyWifiHelper wifiHelper(WIFI_HOSTNAME);


/* ----------------------------------------------------------- */
void setup() {
    
    Serial.begin(9600);
    Serial.println("Booting");
    Serial.println(versionText);

    statusStick.begin();
    statusStick.setPixelColor(0, statusStick.Color(0, 255, 0));
    statusStick.show();

    Wire.begin();

    // debug mode?
    debugMode = inDebugMode();
    if (debugMode) {
    	Serial.println("DEBUG MODE");
    }

    gEM.addListener(channel[WATER_LOW].eventCode, listener_WaterLevel);
    gEM.addListener(channel[COFFEE_SW].eventCode, listener_CoffeeSw);
    
    delay(1000);

    // get init states
    channel[WATER_LOW].state = getI2cChannelState(channel[WATER_LOW]);
    listener_WaterLevel(channel[WATER_LOW].eventCode, channel[WATER_LOW].state);

    channel[COFFEE_SW].state = getI2cChannelState(channel[COFFEE_SW]);
    listener_CoffeeSw(channel[COFFEE_SW].eventCode, channel[COFFEE_SW].state);

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

    delay(100);
}

/*-------------------------------------------------------*/

void serviceWaterLevel() {

    Channel waterCh = channel[WATER_LOW];
    int waterState = getI2cChannelState(waterCh);
    if (stateChanged(waterCh, waterState)) {
        channel[WATER_LOW].prevState = channel[WATER_LOW].state;
        channel[WATER_LOW].state = waterState;
        Serial.println("Water state changed");
        gEM.queueEvent(waterCh.eventCode, waterState);
    }
}

void serviceBrewSwitch() {
            
    Channel brewCh = channel[COFFEE_SW];
    int brewState = getI2cChannelState(brewCh);

    if (stateChanged(brewCh, brewState)) {
        channel[COFFEE_SW].prevState = channel[COFFEE_SW].state;
        channel[COFFEE_SW].state = brewState;
        Serial.println("Brew state changed");
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
   
    //statusStick.show();
}

void initFuelBar() {
    // for (int i=0; i<statusStick.numPixels(); i++) {
    //     statusStick.setPixelColor(i, statusStick.Color(0, 0, 0)); 
    // }
    // statusStick.show();
    // channel[FUEL_BAR].state = 0;
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
void listener_WaterLevel( int event, int state ) {
    Serial.println("listener_WaterLevel");
    setChannelPixel(channel[WATER_LOW]);
}

void listener_CoffeeSw( int event, int state ) {
    Serial.println("listener_CoffeeSw");
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
