#include <Adafruit_NeoPixel.h>
#include <LPD8806.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <EventManager.h>
#include "appconfig.h"
#include "wificonfig.h"

// --- Status Strip ---

#define PIXEL_PIN       15
#define NUMPIXELS       32          // featherwing
Adafruit_NeoPixel statusBlock = Adafruit_NeoPixel(NUMPIXELS, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

#define WATER_LOW   0
#define COFFEE_SW   1
#define STATUS_FLASHING     10

uint32_t STATUS_COLOUR_OFF = statusBlock.Color(0, 0, 0);
uint32_t STATUS_ERROR_COLOUR = statusBlock.Color(255, 0, 0);
uint32_t STATUS_UNKNOWN_COLOUR = statusBlock.Color(0, 0, 0);


#define NUM_CHANNELS    3
Channel channel[NUM_CHANNELS] = {
                        { 
                            WATER_LOW,                      // index
                            statusBlock.Color(0, 0, 127),   // color
                            statusBlock.Color(0, 0, 1),     // colorOff
                            -2,                              // state
                            EventManager::kEventUser0,       // eventCode
                            0                               // onVal
                       },
                        { 
                            COFFEE_SW,                      // index
                            statusBlock.Color(153, 76, 0),  // color
                            statusBlock.Color(2, 1, 0),     // colorOff
                            -2,                              // state
                            EventManager::kEventUser2,       // eventCode
                            1                               // onVal
                        }
                    };

/* ----------------------------------------------------------- */

bool state = 1;
long ticks = 0;

int counter = 1;
bool ledState = false;
long lastToggled = millis();

EventManager gEM;

void serviceEvent(int st);
int getChannelState();
void updateStatusLeds(int statusBit, int val);
void setBlockChunkToColor(int index, int val);
void listener_WaterLevel( int event, int waterlevel );
void listener_Flashing(int event, int statusBit);
void setupOTA(char* host);

/* ----------------------------------------------------------- */
void setup() {
    
    Serial.begin(9600);
    Serial.println("Booting");

    setupOTA("SilviaPID");
    
    Serial.println("Ready");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    statusBlock.begin();

    Wire.begin();

    gEM.addListener( channel[WATER_LOW].eventCode, listener_WaterLevel);
    //gEM.addListener( EventManager::kEventUser1, listener_Flashing);
    gEM.addListener(channel[COFFEE_SW].eventCode, listener_CoffeeSw);
    
    Serial.print("Number of listners: ");
    Serial.println(gEM.numListeners());

    // get init states
    channel[WATER_LOW].state = getChannelState(WATER_LOW);
    listener_WaterLevel(channel[WATER_LOW].eventCode, channel[WATER_LOW].state);
    channel[COFFEE_SW].state = getChannelState(COFFEE_SW);
    listener_CoffeeSw(channel[COFFEE_SW].eventCode, channel[COFFEE_SW].state);
}

/* ----------------------------------------------------------- */

void loop() {
    
    char payload[20];

    ArduinoOTA.handle();

    gEM.processEvent();

    serviceEvent(WATER_LOW);
    //serviceEvent(STATUS_FLASHING);
    serviceEvent(COFFEE_SW);

    delay(100);
}

void serviceEvent(int st) {

    switch (st) {
        
        case WATER_LOW: {
            int waterlevel = getChannelState(WATER_I2C);
            if (channel[WATER_LOW].state != waterlevel) {
                channel[WATER_LOW].state = waterlevel;
                gEM.queueEvent(channel[WATER_LOW].eventCode, waterlevel);
            }
            }
            break;
        case COFFEE_SW: {
            int coffeeSwState = getChannelState(COFFEE_SW_I2C);
            if (channel[COFFEE_SW].state != coffeeSwState) {
                channel[COFFEE_SW].state = coffeeSwState;
                gEM.queueEvent(channel[COFFEE_SW].eventCode, coffeeSwState);
            }
            }
            break;               
        case STATUS_FLASHING: {
            if ((millis() - lastToggled) > 1000) {
                gEM.queueEvent(EventManager::kEventUser1, ledState);
                lastToggled = millis();
            }
            }
            break;
            
    }
}

int getChannelState(int i2caddress) {
    Wire.requestFrom(i2caddress, 1);

    while (Wire.available()) {
        byte c = Wire.read();
        
        Serial.print("Recieved: ");
        if (c == 1) {
            Serial.println("1");
            return 1;
        } else if (c == 0) {
            Serial.println("0");
            return 0;
        } else {
            Serial.println("?");
            return I2C_UNKNOWN_RESPONSE;
        }
    }
    return I2C_NO_RESPONSE;    
}

void updateStatusLeds(int statusBit, int val) {

    switch (statusBit) {
        case WATER_LOW:
            setBlockChunkToColor(WATER_LOW, val);
            statusBlock.show();
            break;
        case COFFEE_SW:
            setBlockChunkToColor(COFFEE_SW, val);
            statusBlock.show();
            break;
    }
}

void setBlockChunkToColor(int index, int val) {
    
    int block = index;
    int onVal = channel[index].onState;
    uint32_t color = val == onVal ? channel[index].color : channel[index].colorOff;
    statusBlock.setPixelColor((block * 2) + 0, color); 
    if (val == I2C_NO_RESPONSE) {
        color = STATUS_ERROR_COLOUR;
    } else if (val == I2C_UNKNOWN_RESPONSE) {
        color = STATUS_UNKNOWN_COLOUR;
    }
    statusBlock.setPixelColor((block * 2) + 1, color); 
    statusBlock.setPixelColor((block * 2) + 8, color); 
    statusBlock.setPixelColor((block * 2) + 9, color); 
}

/* ----------------------------------------------------------- */
void listener_WaterLevel( int event, int state ) {
    Serial.println("Water Level Listener");
        
    Serial.print("Water: "); Serial.println(state);
    updateStatusLeds(WATER_LOW, state);
}

void listener_CoffeeSw( int event, int state ) {
    Serial.println("Coffee Sw Listener");
        
    Serial.print("Coffee_Sw: "); Serial.println(state);
    updateStatusLeds(COFFEE_SW, state);
}

void listener_Flashing(int event, int statusBit) {
    Serial.print("Flashing Listener: "); Serial.println(statusBit);

    int val = ledState == 1 ? channel[WATER_LOW].state : 0;
    updateStatusLeds(WATER_LOW, ledState);
    ledState = ledState == 1 ? 0 : 1;
}

void setupOTA(char* host) {
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.println("Connection Failed! Rebooting...");
        delay(5000);
        ESP.restart();
    }
    
    ArduinoOTA.setHostname(host);
    ArduinoOTA.onStart([]() {
        Serial.println("Start");
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

    ArduinoOTA.begin();
}
