#ifndef _APP_H
#define _APP_H

#include "utils.h"
#include "tinudht.h"
#include <ds18b20/ds18b20.h>
#include "SSD1306_minimal.h"
#include <avr/pgmspace.h>
#include <avr/sleep.h>

#ifdef __cplusplus
extern "C" {
#endif

//#define DEG "\xa7" "C"
#define DEG "\x3b" "\x3c"
#define PERCENT '%'

#define _ERR_MSG "-\072\072\072\072\072\072\072"

#define PAGE_HEIGHT (32/4)
#define PAGE1 (0 * PAGE_HEIGHT)
#define PAGE2 (1 * PAGE_HEIGHT)
#define PAGE3 (2 * PAGE_HEIGHT)
#define PAGE4 (3 * PAGE_HEIGHT)

// button has pull-up resistor
#define BUTTON_PRESSED  0U
#define BUTTON_RELEASED 1U

#define DEBOUNCE_DELAY 50
#define LONG_PRESS_TIME 1000

typedef struct {
	TinuDHT current;
	TinuDHT last;
	uint8_t status;
} TinuDHT_data;

typedef struct {
	float32_t current;
	float32_t last;
	int8_t status;
} DS18B20_data;


void main(void) __attribute__ ((noreturn));
void setup(void);
void loop(void);
static void prepareDisplay_dht11(void);
static void prepareDisplay_ds18b20(void);
static void drawRaindrop(void);
void Timer0_init(void);
void loop_dht11(bool force_update);
void loop_ds18b20(bool force_update);
void fsm_thermomether(void);


// byte array of bitmap 10x32px
static const uint8_t  img_thermometer_cold[] PROGMEM = {
	10,32,
	0x00, 0x00, 0xf0, 0x0c, 0x02, 0x02, 0x0c, 0xf0, 0x20, 0x20, 
	0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0x49, 0x49, 
	0x00, 0x80, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0x92, 0x12, 
	0x1e, 0x61, 0x40, 0x80, 0x80, 0x80, 0x80, 0x40, 0x61, 0x1e
};

// byte array of bitmap 10x32px
static const uint8_t  img_thermometer_warm[] PROGMEM = {
	10,32,
	0x00, 0x00, 0xf0, 0x0c, 0x02, 0x02, 0x0c, 0xf0, 0x20, 0x20, 
	0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0x49, 0x49, 
	0x00, 0x80, 0xff, 0xfc, 0xfc, 0xfc, 0xfc, 0xff, 0x92, 0x12, 
	0x1e, 0x7f, 0x7f, 0xff, 0xff, 0xff, 0xff, 0x7f, 0x7f, 0x1e
};

// byte array of bitmap 10x32px
static const uint8_t  img_thermometer_hot[] PROGMEM = {
	10,32,
	0x00, 0x00, 0xf0, 0xcc, 0xc2, 0xc2, 0xcc, 0xf0, 0x20, 0x20, 
	0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x49, 0x49, 
	0x00, 0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x92, 0x12, 
	0x1e, 0x7f, 0x7f, 0xff, 0xff, 0xff, 0xff, 0x7f, 0x7f, 0x1e
};

// byte array of bitmap 18x32px
/*static const uint8_t img_raindrop[74] PROGMEM = {
	18,32,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xfc, 0xf8, 0xe0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x80, 0xc0, 0xf0, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xf8, 0xe0, 0x80, 0x00, 0x00, 
	0xf8, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf9, 0x03, 0x1e, 0xf8, 
	0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x3f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x3f, 0x3f, 0x19, 0x0c, 0x07, 0x01
};*/

// byte array of bitmap 23x32px
static const uint8_t img_raindrop[94] PROGMEM = {
	23,32,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xf0, 0xf8, 0xfe, 0xff, 0xfc, 0xf0, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xe0, 0xf8, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0x80, 0x00, 0x00, 
	0x78, 0xff, 0xff, 0x07, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x78, 
	0x00, 0x03, 0x0f, 0x1e, 0x38, 0x30, 0x71, 0x63, 0x63, 0xe7, 0xff, 0xff, 0xff, 0xff, 0x7f, 0x7f, 0x7f, 0x3f, 0x1f, 0x1f, 0x0f, 0x03, 0x00
};

#ifdef __cplusplus
}
#endif

#endif // _APP_H