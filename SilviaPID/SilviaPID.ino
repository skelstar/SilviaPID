#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <appconfig.h>

#include <Wire.h>

//#define SoftwareRX    14
//#define SoftwareTX    12
//SoftwareSerial hubSerial(SoftwareRX, SoftwareTX, false, 256);

const char* host = "SilviaPID";

/* ----------------------------------------------------------- */

int stopwatch = 0;

#define led_pin         0
#define LED_ON          LOW
#define LED_OFF         HIGH

/* ----------------------------------------------------------- */
void setup() {
    
    Serial.begin(9600);
    Serial.println("Booting");

    //hubSerial.begin(9600);

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

    delay(100);

    ArduinoOTA.handle();

    String payload = "4321";

    Serial.println(STX + payload + ETX);

    //long start = millis();
    //while (start > millis()-100);    
    
    while (Serial.available() > 0) {
        
        Serial.print(Serial.readString());
    }
}


