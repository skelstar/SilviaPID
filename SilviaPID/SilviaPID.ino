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

byte heart[8] = {
    0b00000,
    0b00000,
    0b01010,
    0b11111,
    0b11111,
    0b01110,
    0b00100,
    0b00000
};

byte coffee1[8] = {
    0b00100,
    0b00100,
    0b10001,
    0b10001,
    0b10001,
    0b10001,
    0b10001,
    0b11111    
};

byte coffee2[8] = {
    0b00000,
    0b00100,
    0b10001,
    0b10001,
    0b10001,
    0b11111,
    0b11111,
    0b11111    
};

byte coffee3[8] = {
    0b00000,
    0b00000,
    0b10001,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111    
};

// Status Strip

#define statusDataPin   0
#define statusClkPin    15
#define statusNumLeds   6

#define NUM_CHANNELS    3
Channel channels[NUM_CHANNELS] = {
                        { 0, OFF, 0 }, // WATER
                        { 0, OFF, 0 }, // COFFEE
                        { 0, OFF, 0 }  // HEATING
                      };

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

    lcd.begin(16, 2);
    lcd.setRGB(255, 0, 0);
#if 1
    lcd.createChar(0, heart);
    lcd.createChar(1, coffee1);
    lcd.createChar(2, coffee2);
    lcd.createChar(3, coffee3);
#endif
//             0123456789012345
    lcd.print("   Silvia PID   ");

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

    getWaterLevel(payload);

    //getInputsFromHub(payload);
    
    //processPacket(payload);

    if (stateChanged())
        toggleLcdOnlineChar(state);

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
        } else if (c == 0) {
            Serial.println("0");
        } else {
            Serial.println("?");
        }
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
                lcd.setCursor(3, LCD_ROW_BOTTOM);
                lcd.print("Water Low");
                lcd.setRGB(0, 0, 255);  // BLUE
                
            } else {
                lcd.setCursor(3, LCD_ROW_BOTTOM);
                lcd.print("         ");
                lcd.setRGB(0, 255, 0);  // GREEN
            }
        }
        else {
        }
    }
    return;    
}

void toggleLcdOnlineChar(bool state) {
    
    lcd.setCursor(15, LCD_ROW_BOTTOM);
    if (state == 1) {
        lcd.write((unsigned char)0);
    } else {
        lcd.print(' ');
    }

    lcd.setCursor(14, LCD_ROW_BOTTOM);
    if (state == 1) {
        if (counter == 1) {
            lcd.write(1);
        } else if (counter == 2) {
            lcd.write(2);
        } else if (counter == 3) {
            lcd.write(3);
        } else if (counter > 3) {
            lcd.print(' ');
            counter = -1;
        }
        counter++;
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
