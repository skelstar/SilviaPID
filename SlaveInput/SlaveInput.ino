#include <TinyWireS.h>

int led_pin = 1;
int input_pin = 3;
int I2C_SLAVE_ADDR = 0x26;

#define LED_ON  HIGH
#define LED_OFF  LOW

/* ---------------------------------------------- */

void setup() {
  
    pinMode(led_pin, OUTPUT);
    pinMode(input_pin, INPUT);
    digitalWrite(led_pin, LED_ON);
    TinyWireS.begin(I2C_SLAVE_ADDR);
    TinyWireS.onRequest(requestEvent);
}

/* ---------------------------------------------- */

void loop() {

    TinyWireS_stop_check();
}

/* ---------------------------------------------- */

void requestEvent()
{  
    digitalWrite(led_pin, LED_ON);
    int val = digitalRead(input_pin);
    TinyWireS.send(val);
    digitalWrite(led_pin, LED_OFF);
}


