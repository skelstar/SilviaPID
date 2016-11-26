#include <LPD8806.h>
#include <rgb_lcd.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <EventManager.h>
#include "appconfig.h"
#include "wificonfig.h"

#include <Wire.h>

// RGB LCD

rgb_lcd lcd;

// Status Strip

#define statusDataPin   0
#define statusClkPin    15
#define statusNumLeds   6

LPD8806 strip = LPD8806(statusNumLeds, statusDataPin, statusClkPin);

#define NUM_CHANNELS    3
Channel channels[NUM_CHANNELS] = {
                        { strip.Color(0, 127, 0), OFF, 0 }, // WATER
                        { strip.Color(0, 0, 127), OFF, 0 }, // COFFEE
                        { strip.Color(127, 0, 0), OFF, 0 }  // HEATING
                      };

uint32_t BLANK_COLOUR = strip.Color(0, 0, 0);

/* ----------------------------------------------------------- */

#define led_pin         0
#define LED_ON          LOW
#define LED_OFF         HIGH

/* ----------------------------------------------------------- */
void setup() {

    strip.begin();
    strip.show();
    
    Serial.begin(9600);
    Serial.println("Booting");

    lcd.begin(16, 2);
    lcd.setRGB(255, 0, 0);

//             0123456789012345
    lcd.print("   Silvia PID");

    setupOTA("SilviaPID");

    pinMode(led_pin, OUTPUT);
    digitalWrite(led_pin, LED_ON);
    
    Serial.println("Ready");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    Wire.begin();
}

/* ----------------------------------------------------------- */

void loop() {
    
    char payload[20];

    ArduinoOTA.handle();

    setLedOn(false);

    transmitPacket("XXX00");

    //delay(100);

    String packet;

    delay(100);        
    
    packet = receiveResponse();
    
    packet.trim();
     
    if (packet != "") {
        if (isValidPacket(packet)) {
            getPayload(packet, payload, PAYLOAD_SIZE);
            Serial.println(payload);
            processPacket(payload);
        } else {
            Serial.print("Not valid: '"); Serial.print(packet); Serial.println("'");
        }            
    }

    delay(900);
}

void transmitPacket(String command) {
    
    Serial.print(STX); Serial.print(command); Serial.println(ETX);
    Serial.flush();
}

String receiveResponse() {

    String packet = "";
    while (Serial.available() > 0) {
        //delay(10);
        char character = Serial.read();
        packet += character;
        setLedOn(true);
    }
    Serial.flush();
    return packet;
}

bool isValidPacket(String packet) {
    if (packet.length() > 0 &&
        packet.indexOf(ACK) >= 0 && 
        packet.indexOf(ETX) >= 3)
        return true;
    return false;       
}

void getPayload(String packet, char* result, int resultSize) {
    int start = packet.indexOf(ACK) + 3;
    int end1 = packet.indexOf(ETX);

    String pl = packet.substring(start, end1);
    pl.toCharArray(result, resultSize);    
}

void processPacket(String packet) {
    
    if (packet == "")
        return; 

    for (int i = 0; i < NUM_CHANNELS; i++) {

        if (i == WATER) {
            if (packet[i] == ON) {
                lcd.setCursor(1, 0);
                lcd.print("Water Low");
            } else {
                lcd.setCursor(1, 0);
                lcd.print("         ");
            }
        }
        else {
//            if (packet[i] == ON) {
//                strip.setPixelColor(i, channels[i].color);
//            else if (packet[i] == ON) {
//                strip.setPixelColor(i, channels[i].color);
//            } else {
//                strip.setPixelColor(i, BLANK_COLOUR);
//            }
        }
    }
    strip.show();
    
    return;    
}

void blankStrip() {
    for (int i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, 0, 0, 0);
    }
    strip.show();    
}

void setLedOn(bool on) {
    if (on) {
        digitalWrite(led_pin, LED_ON);
    }
    else {
        digitalWrite(led_pin, LED_OFF);
    }
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
