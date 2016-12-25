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

#include <Wire.h>

// --- Status Strip ---

#define PIN   15
#define NUMPIXELS   8
Adafruit_NeoPixel statusStrip = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ400);

#define STATUS_WATER_LEVEL  0
#define STATUS_BUS_ERROR    1
//#define STATUS_
//#define STATUS_
//#define STATUS_

uint32_t STATUS_COLOUR_OFF = statusStrip.Color(0, 0, 0);
uint32_t STATUS_WATER_LEVEL_COLOUR = statusStrip.Color(0, 0, 255);
uint32_t STATUS_BUS_ERROR_COLOUR = statusStrip.Color(255, 0, 0);


#define NUM_CHANNELS    3
Channel channels[NUM_CHANNELS] = {
                        { 0, OFF, 0 }, // WATER
                        { 0, OFF, 0 }, // COFFEE
                        { 0, OFF, 0 }  // HEATING
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

    statusStrip.begin();

    Wire.begin();
}

/* ----------------------------------------------------------- */

void loop() {
    
    char payload[20];

    ArduinoOTA.handle();

    int waterlevel = getWaterLevel(payload);
    if (waterlevel == -1) {
        setStatus(STATUS_BUS_ERROR, 0);
        waterlevel = getDebugWaterLevel();
    }
    Serial.print("Water: "); Serial.println(waterlevel);
    setStatus(STATUS_WATER_LEVEL, waterlevel);
    
    //getInputsFromHub(payload);
    
    //processPacket(payload);

    delay(200);
}

int getWaterLevel(char* reg) {

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
}

int getDebugWaterLevel() {
    pinMode(WATER_LEVEL_DEBUG_PIN, INPUT);
    digitalWrite(WATER_LEVEL_DEBUG_PIN, HIGH);  // turn on pull-up resistor
    return digitalRead(WATER_LEVEL_DEBUG_PIN);
}

void setStatus(int statusBit, int val) {

    switch (statusBit) {
        case STATUS_WATER_LEVEL:
            if (val == 1) {
                statusStrip.setPixelColor(STATUS_WATER_LEVEL, STATUS_WATER_LEVEL_COLOUR);
            }
            else {
                statusStrip.setPixelColor(STATUS_WATER_LEVEL, STATUS_COLOUR_OFF);
            }
            statusStrip.show();
            break;
        case STATUS_BUS_ERROR:
            statusStrip.setPixelColor(STATUS_BUS_ERROR, STATUS_BUS_ERROR_COLOUR);
            statusStrip.show();
            break;
    }
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
