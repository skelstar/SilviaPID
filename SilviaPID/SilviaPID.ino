#include <LPD8806.h>
#include <rgb_lcd.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <EventManager.h>
#include "appconfig.h"
#include "wificonfig.h"

#include <Wire.h>

const char* host = "SilviaPID";

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

SoftwareSerial hubSerial(12, 13);   // RX, TX

/* ----------------------------------------------------------- */
void setup() {

    strip.begin();
    strip.show();
    
    Serial.begin(115200);
    Serial.println("Booting");

    lcd.begin(16, 2);
    lcd.setRGB(0, 0, 255);

    lcd.print("hello World");
    lcd.blink();

    hubSerial.begin(9600);

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

    delay(100);

    String packet = receiveResponse();

    packet.trim();
    if (packet != "") {
        if (isValidPacket(packet)) {
            getPayload(packet, payload, PAYLOAD_SIZE);
            Serial.println(payload);
            processPacket(payload);
        }
    }
}

void transmitPacket(String command) {
    
    hubSerial.print(STX); hubSerial.print(command); hubSerial.print(ETX);
    hubSerial.flush();
}

String receiveResponse() {

    String packet = "";
    while (hubSerial.available() > 0) {
        delay(3);
        char character = hubSerial.read();
        packet += character;
        setLedOn(true);
    }
    hubSerial.flush();
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
        if (packet[i] == ON) {
            strip.setPixelColor(i, channels[i].color);
        } else {
            strip.setPixelColor(i, BLANK_COLOUR);
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


