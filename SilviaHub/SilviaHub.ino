//#include <ESP8266WiFi.h>
//#include <ESP8266mDNS.h>
//#include <WiFiUdp.h>
//#include <ArduinoOTA.h>
#include <appconfig.h>

void setup() {

    Serial.begin(115200);
    Serial.println();
    Serial.println("Start!");

    delay(1000);

    pinMode(2, OUTPUT);
    digitalWrite(2, LOW);
}

void loop() {
    String packet;
    String payload;
    
    while (Serial.available() > 0) {
        packet = Serial.readString();
    }

    if (packet.length() > 0) {
        if (isValidPacket(packet)) {
            Serial.println("Valid!");
            Serial.print("Payload: ");
            Serial.println(getPayload(packet));
        }
        else {
            Serial.println("ERROR");
        }
    }
}

bool isValidPacket(String packet) {
    if (packet.startsWith(STX) && 
        packet.endsWith(ETX))
        return true;
    return false;       
}

String getPayload(String packet) {
    int start = packet.indexOf(STX) + 3;
    int end1 = packet.indexOf(ETX);
    return packet.substring(start, end1);
}

