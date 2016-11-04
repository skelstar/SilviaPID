#include <Wire.h>

int led = 1;
int deviceID = 8;

/* ---------------------------------------------- */

void setup() {
  
    pinMode(led, OUTPUT);
    Wire.begin(deviceID);
    Wire.onReceive(receiveEvent)
}

/* ---------------------------------------------- */

void loop() {

//    digitalWrite(led, HIGH); 
//    delay(2000);
//    digitalWrite(led, LOW);
//    delay(1000);
}

/* ---------------------------------------------- */

void receiveEvent(int howMany) {
    while (1 < Wire.available()) {
        char c = Wire.read();
    }
    int x = Wire.read();
}
    
