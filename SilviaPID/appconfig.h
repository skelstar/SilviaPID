
const char* STX = "STX";
const char* ETX = "ETX";
const char* ENQ = "ENQ";
const char* ACK = "ACK";

const char ON	= '1';
const char OFF	= '0';
const char NA	= '-';

#define LCD_ROW_TOP     0
#define LCD_ROW_BOTTOM  1

#define MASTER_I2C      1
#define HUB_I2C         8
#define WATER_I2C       9
#define PAYLOAD_SIZE    5

// payload register
#define WATER		0   // BLUE
#define COFFEE 	    1   // GREEN
#define HEATING	    2   // RED
#define PUMP			3
#define BOILER			4

//#define STEAMSWITCH 	2
//#define WATERSWITCH 	3


struct Channel {
    //int index;
    uint32_t color;
    uint32_t colorOff;
    int state;
    int pin;
    int index;
};


