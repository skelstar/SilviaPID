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

/* ----------------------------------------------------------- */

int stopwatch = 0;

#define led_pin         0
#define LED_ON          LOW
#define LED_OFF         HIGH

SoftwareSerial hubSerial(12, 13);   // RX, TX

/* ----------------------------------------------------------- */
void setup() {
    
    Serial.begin(115200);
    Serial.println("Booting");

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

    ArduinoOTA.handle();

    String packet = "";
    char character;
    char payload[20];

    while (hubSerial.available() > 0) {
        delay(3);
        character = hubSerial.read();
        packet += character;
        setLedOn(true);
    }
    hubSerial.flush();

    packet.trim();
    if (packet != "") {
        Serial.println(packet);
    }
    
    //Serial.flush();
    //delay(400);

    setLedOn(false);
    String cmd = "XXX00";

    hubSerial.print(STX); hubSerial.print(cmd); hubSerial.print(ETX);
    hubSerial.flush();

    delay(100);
}

void setLedOn(bool on) {
    if (on) {
        digitalWrite(led_pin, LED_ON);
    }
    else {
        digitalWrite(led_pin, LED_OFF);
    }
}


