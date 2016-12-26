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

#define STATUS_WATER_LEVEL  0
//#define STATUS_BUS_ERROR    1
#define STATUS_FLASHING     1

uint32_t STATUS_COLOUR_OFF = statusBlock.Color(0, 0, 0);
uint32_t STATUS_WATER_LEVEL_COLOUR = statusBlock.Color(0, 0, 255);
uint32_t STATUS_ERROR_COLOUR = statusBlock.Color(255, 0, 0);


#define NUM_CHANNELS    3
Channel channel[NUM_CHANNELS] = {
                        { 
                            statusBlock.Color(0, 0, 255), 
                            statusBlock.Color(0, 0, 1), 
                            -2, 
                            0, 
                            STATUS_WATER_LEVEL 
                        }, // WATER
                        { 0, -2, 0, 1 }, // COFFEE
                        { 0, -2, 0, 2 }  // HEATING
                      };

/* DEBUG MODE */
#define WATER_LEVEL_DEBUG_PIN   16                      

/* ----------------------------------------------------------- */

#define led_pin         0
#define LED_ON          LOW
#define LED_OFF         HIGH

bool state = 1;
long ticks = 0;

int counter = 1;
bool ledState = false;
long lastToggled = millis();

EventManager gEM;

void serviceEvent(int st);
int getWaterLevel();
void setStatus(int statusBit, int val);
void setBlockChunkToColor(int index, int val);
void listener_WaterLevel( int event, int waterlevel );
void listener_Flashing(int event, bool ledSt);
void setupOTA(char* host);

/* ----------------------------------------------------------- */
void setup() {
    
    Serial.begin(9600);
    Serial.println("Booting");

    setupOTA("SilviaPID");

    pinMode(led_pin, OUTPUT);
    digitalWrite(led_pin, LED_ON);
    
    Serial.println("Ready");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    statusBlock.begin();

    Wire.begin();

    gEM.addListener( EventManager::kEventUser0, listener_WaterLevel);
    gEM.addListener( EventManager::kEventUser1, listener_Flashing);
    Serial.print("Number of listners: ");
    Serial.println(gEM.numListeners());

    channel[STATUS_WATER_LEVEL].state = getWaterLevel();
}

/* ----------------------------------------------------------- */

void loop() {
    
    char payload[20];

    ArduinoOTA.handle();

    gEM.processEvent();

    serviceEvent(STATUS_WATER_LEVEL);
    serviceEvent(STATUS_FLASHING);

    delay(200);
}

void serviceEvent(int st) {

    switch (st) {
        
        case STATUS_WATER_LEVEL: {
                int waterlevel = getWaterLevel();
                if (channel[STATUS_WATER_LEVEL].state != waterlevel) {
                    channel[STATUS_WATER_LEVEL].state = waterlevel;
                    gEM.queueEvent(EventManager::kEventUser0, waterlevel);
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

int getWaterLevel() {

    Wire.requestFrom(WATER_I2C, 1);

    int i = 0;
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
            return -1;
        }
    }
    return -1;
}

void setStatus(int statusBit, int val) {

    switch (statusBit) {
        case STATUS_WATER_LEVEL:
            setBlockChunkToColor(STATUS_WATER_LEVEL, val);
            statusBlock.show();
            break;
    }
}

void setBlockChunkToColor(int index, int val) {
    int block = index;
    uint32_t color = val == 1 ? channel[index].color : channel[index].colorOff;
    statusBlock.setPixelColor((block * 2) + 0, color); 
    if (val == -1) {
        color = STATUS_ERROR_COLOUR;
    }
    statusBlock.setPixelColor((block * 2) + 1, color); 
    statusBlock.setPixelColor((block * 2) + 8, color); 
    statusBlock.setPixelColor((block * 2) + 9, color); 
}

/* ----------------------------------------------------------- */
void listener_WaterLevel( int event, int waterlevel ) {
    Serial.println("Water Level Listener");
        
    Serial.print("Water: "); Serial.println(waterlevel);
    setStatus(STATUS_WATER_LEVEL, waterlevel);
}

void listener_Flashing(int event, int ledSt) {
    Serial.print("Flashing Listener: "); Serial.println(ledState);

    int val = ledState == 1 ? channel[STATUS_WATER_LEVEL].state : 0;
    setStatus(STATUS_WATER_LEVEL, ledState);
    ledState = ledState == 1 ? 0 : 1;
}

bool stateChanged() {
    
    if (ticks < millis() - 500) {
        state = !state;
        ticks = millis();
        return true;
    }
    return false;
}

void getInputsFromHub(char* reg) {

    Wire.requestFrom(HUB_I2C, PAYLOAD_SIZE);
    char payload[PAYLOAD_SIZE];

    int i = 0;
    while (Wire.available()) {
        char c = Wire.read();
        if (i == 0)
            Serial.print("Payload: '");
        Serial.print(c);
        reg[i++] = c;
    }
    if (i > 0)
        Serial.println("' end");
}

void processPacket(String packet) {
    
    if (packet == "")
        return; 

    for (int i = 0; i < NUM_CHANNELS; i++) {

        if (i == WATER) {
            if (packet[i] == ON) {
//                lcd.setCursor(3, LCD_ROW_BOTTOM);
//                lcd.print("Water Low");
//                lcd.setRGB(0, 0, 255);  // BLUE
            } else {
//                lcd.setCursor(3, LCD_ROW_BOTTOM);
//                lcd.print("         ");
//                lcd.setRGB(0, 255, 0);  // GREEN
            }
        }
        else {
        }
    }
    return;    
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
