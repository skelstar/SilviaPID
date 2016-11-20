#include <appconfig.h>
#include <SoftwareSerial.h>
#include <ctype.h>

#define LED_PIN 2

char inData[20]; // Allocate some space for the string
char inChar=-1; // Where to store the character read
byte idx = 0; // Index into array; where to store the character


SoftwareSerial swSer(D5, D6, false, 256);

void setup() {

    Serial.begin(9600);
    Serial.println();
    Serial.println("Start!");

    swSer.begin(9600);

    
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);
}

void loop() {
    String packet = "";
    char character;

    while (swSer.available() > 0) {
        delay(3);
        character = swSer.read();
        packet += character;
    }

    packet.trim();
    if (packet != "") {
        if (isValidPacket(packet)) {
            Serial.print("Valid");
            Serial.print("Payload: ");
            Serial.println(getPayload(packet));
        }
        else {
            Serial.println(packet);
        }
        flashLed();
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

String getPayload(String packet) {
    int start = packet.indexOf(STX) + 3;
    int end1 = packet.indexOf(ETX);
    return packet.substring(start, end1);
}

