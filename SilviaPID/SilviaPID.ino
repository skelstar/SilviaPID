#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "appconfig.h"

#include <Wire.h>
#include <U8g2lib.h>

/* ----------------------------------------------------------- */

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, SCL, SDA, U8X8_PIN_NONE);

#define font_stopwatch u8g2_font_logisoso42_tf
int stopwatch = 0;

/* ----------------------------------------------------------- */
void setup() {
    Serial.begin(115200);
    Serial.println("Booting");

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.println("Connection Failed! Rebooting...");
        delay(5000);
        ESP.restart();
    }
    
    ArduinoOTA.setHostname(host);
    //ArduinoOTA.setPassword((const char *)"123");
    
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
    
    Serial.println("Ready");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    Wire.begin();
    u8g2.begin();  

  
}

/* ----------------------------------------------------------- */

void loop() {
    if (stopwatch >= 0) {
        u8g2.clearBuffer();                    // clear the internal menory
        u8g2.setFont(font_stopwatch);
        u8g2.drawStr(0, 42, String(stopwatch--, DEC)); // write something to the internal memory
        u8g2.sendBuffer();                    // transfer internal memory to the display
    }

    delay(1000);

    ArduinoOTA.handle();
}
