#include <TinyWireS.h>
#include "appconfig.h"

#define INPUT_PIN   1   // Trinket is pin 3 (1 is tied to LED)

bool i2cReadEvent = false;
int val = 0;

void setup() {

    TinyWireS.begin(COFFEE_SW_I2C);
    TinyWireS.onRequest(requestEvent);

    pinMode(INPUT_PIN, INPUT);
    digitalWrite(INPUT_PIN, HIGH);  // turn on pull-up resistor
}

void loop() {

    delay(100);

    TinyWireS_stop_check();
}

void requestEvent() {

    val = digitalRead(INPUT_PIN);
    TinyWireS.send(val);
    i2cReadEvent = true;
}

