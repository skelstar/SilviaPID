#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <appconfig.h>

const char* host = "SilviaHub";

void setup() {

    Serial.begin(9600);
    Serial.println();
    Serial.println("Start!");

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
    
    pinMode(2, OUTPUT);
    digitalWrite(2, HIGH);
}

void loop() {
    String packet;
    String payload;
    
    while (Serial.available() > 0) {
        packet = Serial.readString();
    }

    if (packet.length() > 0)
    {
        digitalWrite(2, LOW);
        delay(100);
        digitalWrite(2, HIGH);
        
        if (isValidPacket(packet)) {
    
            Serial.print(ACK + getPayload(packet) + ETX);
            digitalWrite(2, LOW);
            delay(100);
            digitalWrite(2, HIGH);
        }
        else {
            Serial.print(" ERROR: "); Serial.println(packet);
        }
    }
    ArduinoOTA.handle();
}

bool isValidPacket(String packet) {
    packet.trim();
    if (packet.length() > 0 &&
        packet.indexOf(STX) >= 0 && 
        packet.indexOf(ETX) >= 3)
        return true;
    return false;       
}

String getPayload(String packet) {
    int start = packet.indexOf(STX) + 3;
    int end1 = packet.indexOf(ETX);
    return packet.substring(start, end1);
}

