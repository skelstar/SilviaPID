
const char* STX = "STX";
const char* ETX = "ETX";
const char* ENQ = "ENQ";
const char* ACK = "ACK";

const char ON	= '1';
const char OFF	= '0';
const char NA	= '-';

// payload register
#define WATER_LEVEL		0   // BLUE
#define COFFEE_SWITCH 	1   // GREEN
#define HEATING_LIGHT	2   // RED
#define PUMP			3
#define BOILER			4

//#define STEAMSWITCH 	2
//#define WATERSWITCH 	3

struct Channel {
    //int index;
    uint32_t color;
    char state;
    int pin;
};

