#ifndef _APP_H
#define _APP_H

#include "utils.h"
#include "bitmaps.h"
#include "DHT11.h"
#include "ds18b20/ds18b20.h"
#include "SSD1306_minimal.h"
#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

#ifdef __cplusplus
extern "C" {
#endif

//#define DEG "\xa7" "C"
#define DEG "\x3b" "\x3c"
#define PERCENT '%'

#define _ERR_MSG "- ERROR "

#define PAGE_HEIGHT (32/4)
#define PAGE1 (0 * PAGE_HEIGHT)
#define PAGE2 (1 * PAGE_HEIGHT)
#define PAGE3 (2 * PAGE_HEIGHT)
#define PAGE4 (3 * PAGE_HEIGHT)

// button has pull-up resistor
#define BUTTON_PRESSED  0U
#define BUTTON_RELEASED 1U

#define DEBOUNCE_DELAY    50 /* time in ms */
#define LONG_PRESS_TIME 1500 /* time in ms */

typedef struct {
	DHT current;
	DHT last;
	uint8_t status;
} DHT_data;

typedef struct {
	float32_t current;
	float32_t last;
	int8_t status;
} DS18B20_data;

typedef enum {SCREEN_OFF, SCREEN_DHT11, SCREEN_DS18B20, SCREEN_MAX} screen_state_t;

void main(void) __attribute__ ((noreturn));
void uC_init(void);
void appSetup(void);
void loop(void);
void prepareDisplay_dht11(void);
void prepareDisplay_ds18b20(void);
void loop_dht11(void);
void loop_ds18b20(void);
uint8_t getScreenState(void);
void buttonSimpleAction(void);
void buttonLongPressed(void);
void appLoop(void);
void screenOFF(void);



// ATtiny led on PB1
//#define tiny_led  PB1
//#define LED_PIN  tiny_led
#define BUTTON_PIN PB1

//----- DHT Configuration ----------------------//
#define DHT_Type	DHT11      //DHT11 or DHT22
#define DHT_Pin		PB3
#define TINUDHT_PIN DHT_Pin
//----------------------------------------------//

// ds18b20 pin
#define DS18B20_PIN PB4
#define DS18B20_pinMask (1U << DS18B20_PIN)

// Software Serial Pins
#define RX          PB0
#define TX          PB2

// I2C Pins
#define SDA         PB0
#define SCL         PB2





#ifdef __cplusplus
}
#endif

#endif // _APP_H