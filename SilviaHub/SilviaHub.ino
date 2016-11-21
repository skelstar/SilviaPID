#include "appconfig.h"
#include "wificonfig.h"
#include <ctype.h>

#define LED_PIN 2

void setup() {

    Serial.begin(9600);
    Serial.println();
    Serial.println("Start!");

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);
}

void loop() {
    String packet = "";
    char character;
    char payload[20];

    while (Serial.available() > 0) {
        delay(3);
        character = Serial.read();
        packet += character;
    }

    packet.trim();
    if (packet != "") {
        if (isValidPacket(packet)) {
            readPayload(packet, payload, sizeof(payload));
            getInputs(payload);
            sendResponse(payload);
            flashLed();
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
    Serial.print(ACK); Serial.print(payload); Serial.println(ETX);
}

void getInputs(char* reg) {
    reg[WATERLEVEL] = getInput(WATERLEVEL);
    reg[COFFEESWITCH] = getInput(COFFEESWITCH);
    reg[HEATINGLIGHT] = getInput(HEATINGLIGHT);
    reg[PUMP] = getInput(PUMP);
    reg[BOILER] = getInput(BOILER);
}

char getInput(int input) {
    switch (input) {
        case WATERLEVEL: return ON;
        case COFFEESWITCH: return ON;
        case HEATINGLIGHT: return OFF;
        case PUMP: return NA;
        case BOILER: return NA;
    }
}

