#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <appconfig.h>

#include <Wire.h>
//#include <U8g2lib.h>

#include "LPD8806.h"
#include "SPI.h"

/* ----------------------------------------------------------- */

//U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, SCL, SDA, U8X8_PIN_NONE);

//#define font_stopwatch u8g2_font_logisoso42_tf
int stopwatch = 0;

#define led_pin         0
#define LED_ON          LOW
#define LED_OFF         HIGH

#define I2C_SLAVE_ADDR  0x26            // i2c slave address (38)
#define QUERY_ALIVE     1

// STRIP
#define STRIP_Data      13
#define STRIP_Clk       12
#define STRIP_BRIGHTNESS    127
#define STRIP_COLOR_WHITE   strip.Color(STRIP_BRIGHTNESS, STRIP_BRIGHTNESS,  STRIP_BRIGHTNESS)
#define STRIP_COLOR_RED     strip.Color(STRIP_BRIGHTNESS, 0, 0)
#define STRIP_COLOR_GREEN   strip.Color(0, STRIP_BRIGHTNESS, 0)
int NUM_PIXELS = 4;

LPD8806 strip = LPD8806(NUM_PIXELS, STRIP_Data, STRIP_Clk);

/* ----------------------------------------------------------- */
void setup() {

    strip.begin();
    strip.show();       // all OFF  
    ShowLightsAllOneColour(STRIP_COLOR_WHITE);
    
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
}

void ShowLightsAllOneColour(uint32_t c) {

    for (int i=0; i<NUM_PIXELS; i++) {
        strip.setPixelColor(i, c);
    }
    strip.show();
}

