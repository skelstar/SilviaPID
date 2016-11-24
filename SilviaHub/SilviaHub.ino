#include "appconfig.h"
#include <ctype.h>

#define LED_PIN             D4

#define WATER_LEVEL_PIN     D0
#define COFFEE_SWITCH_PIN   D5
#define HEATING_LIGHT_PIN   D6

void setup() {

    Serial.begin(9600);
    Serial.println();
    Serial.println("Start!");

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

