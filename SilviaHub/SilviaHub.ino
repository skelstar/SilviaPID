//#include <ESP8266WiFi.h>
//#include <ESP8266mDNS.h>
//#include <WiFiUdp.h>
//#include <ArduinoOTA.h>
#include <appconfig.h>

void setup() {

    Serial.begin(115200);

    delay(1000);

    pinMode(2, OUTPUT);
    digitalWrite(2, LOW);
}

void loop() {
  
    Serial.println("Test");
    delay(1000);
}
