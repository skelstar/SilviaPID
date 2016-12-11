#include <Wire.h>
#include "appconfig.h"

#define INPUT_PIN   12

bool report = false;
int val = 0;

void setup() {

    Serial.begin(115200);
    delay(10);
    Serial.println("Running...");

    Wire.begin(WATER_I2C);
    Wire.onRequest(requestEvent);

    pinMode(INPUT_PIN, INPUT);
    digitalWrite(INPUT_PIN, HIGH);  // turn on pull-up resistor
}

void loop() {

    delay(200);

    if (report) {
        Serial.print("Sending "); Serial.println(val);
    }
}

void requestEvent() {

    val = digitalRead(INPUT_PIN);
    Wire.write(val);
    report = true;
}

