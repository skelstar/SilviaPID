#include "appconfig.h"
#include "wificonfig.h"

#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#define LED_PIN             0

#define WATER_LEVEL_PIN     14
#define COFFEE_SWITCH_PIN   12
#define HEATING_LIGHT_PIN   13

SoftwareSerial pidSerial(12, 13);   // RX, TX

void setup() {

    pidSerial.begin(9600);
    Serial.begin(115200);
    Serial.println("Silvia HUB Start!");

    setupOTA("SilviaHUB");

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);

    pinMode(WATER_LEVEL_PIN, INPUT);
    pinMode(COFFEE_SWITCH_PIN, INPUT);
    pinMode(HEATING_LIGHT_PIN, INPUT);
}

void loop() {
    String packet = "";
    char character;
    char payload[20];

    while (pidSerial.available() > 0) {
        delay(3);
        character = pidSerial.read();
        packet += character;
    }

    packet.trim();
    if (packet != "") {
        if (isValidPacket(packet)) {
            Serial.print(packet);
            readPayload(packet, payload, sizeof(payload));
            getInputs(payload);
            sendResponse(payload);
            flashLed();
        } else {
            Serial.print(packet);
            Serial.println(" : invalid");
        }
    }
}

void flashLed() {
   digitalWrite(LED_PIN, LOW);
   delay(100);
   digitalWrite(LED_PIN, HIGH);
}

bool isValidPacket(String packet) {
    if (packet.length() > 0 &&
        packet.indexOf(STX) >= 0 && 
        packet.indexOf(ETX) >= 3)
        return true;
    return false;       
}

void readPayload(String packet, char* result, int resultSize) {
    int start = packet.indexOf(STX) + 3;
    int end1 = packet.indexOf(ETX);

    String pl = packet.substring(start, end1);
    pl.toCharArray(result, resultSize);    
}

void sendResponse(char* payload) {
    getInputs(payload);
    Serial.print(" -> sending: ");
    pidSerial.print(ACK); pidSerial.print(payload); pidSerial.println(ETX);
}

void getInputs(char* reg) {
    reg[WATER_LEVEL] = getInput(WATER_LEVEL);
    reg[COFFEE_SWITCH] = getInput(COFFEE_SWITCH);
    reg[HEATING_LIGHT] = getInput(HEATING_LIGHT);
    reg[PUMP] = getInput(PUMP);
    reg[BOILER] = getInput(BOILER);
}

char getInput(int input) {
    switch (input) {
        case WATER_LEVEL: 
            return parseInput(digitalRead(WATER_LEVEL_PIN));
        case COFFEE_SWITCH: 
            return parseInput(digitalRead(COFFEE_SWITCH_PIN));
        case HEATING_LIGHT: 
            return parseInput(digitalRead(HEATING_LIGHT_PIN));
        case PUMP: 
            return NA;
        case BOILER: 
            return NA;
    }
}

char parseInput(bool result) {
    if (result == HIGH)
        return ON;
     else 
        return OFF;
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

