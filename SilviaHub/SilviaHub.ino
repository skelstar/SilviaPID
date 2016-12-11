#include <TinyWireS.h>
#include "appconfig.h"


#define LED_PIN         1
#define LED_ON          HIGH
#define LED_OFF         LOW

#define NUM_CHANNELS    3
Channel channels[NUM_CHANNELS] = {
                        { 0, OFF, 3 }, // WATER
                        { 0, OFF, 1 }, // COFFEE
                        { 0, OFF, 4 }  // HEATING
                      };

char payload[PAYLOAD_SIZE];

void setup() {

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LED_OFF);

    pinMode(channels[WATER].pin, INPUT);
    pinMode(channels[COFFEE].pin, INPUT);
    pinMode(channels[HEATING].pin, INPUT);

    TinyWireS.begin(HUB_I2C);
    TinyWireS.onRequest(requestEvent);
}

void loop() {

    getInputs(payload);
    flashLed();
    delay(200);

    TinyWireS_stop_check();
}

void requestEvent() {

    for (int i=0; i<PAYLOAD_SIZE; i++) {
        TinyWireS.send(payload[i]);
    }
}

void flashLed() {
   digitalWrite(LED_PIN, LED_ON);
   delay(50);
   digitalWrite(LED_PIN, LED_OFF);
}

void getInputs(char* reg) {
    reg[WATER] = getInput(WATER);
    reg[COFFEE] = getInput(COFFEE);
    reg[HEATING] = getInput(HEATING);
    reg[PUMP] = getInput(PUMP);
    reg[BOILER] = getInput(BOILER);
}

char getInput(int input) {
    switch (input) {
        case WATER: 
            return parseInput(digitalRead(channels[WATER].pin));
        case COFFEE: 
            return parseInput(digitalRead(channels[COFFEE].pin));
        case HEATING: 
            return parseInput(digitalRead(channels[HEATING].pin));
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

