#ifndef _UTILS_H
#define _UTILS_H

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <stdbool.h>

#ifdef __cplusplus
#include "ifloat32_t.h"

extern "C" {
#endif

#define FIX_POINTER(_ptr) __asm__ __volatile__("" : "=b" (_ptr) : "0" (_ptr))

#define pin_set(n)    (PORTB |=  (1U << (n)))
#define pin_clear(n)  (PORTB &= ~(1U << (n)))

// note: toggling a pin that is set as an input causes to toggle it's pull-up resistor
#define pin_toggle(n) (PINB |= (1U << (n)))

#define pin_get(n)    ((PINB >> (n)) & 1U)

#define pinmode_output(n) (DDRB |=  (1U << (n)))
#define pinmode_input(n)  (DDRB &= ~(1U << (n)))

// note: pull-up can only be set when the corresponding pin is set as an input
#define pinmode_pullup_on(n)  (PORTB |=  (1U << (n)))
#define pinmode_pullup_off(n) (PORTB &= ~(1U << (n)))

//General use bit manipulating commands
#define BitSet(    x, y)			(  (x) |=  (1UL << (y)) )
#define BitClear(  x, y)			(  (x) &= ~(1UL << (y)) )
#define BitToggle( x, y)			(  (x) ^=  (1UL << (y)) )
#define BitCheck(  x, y)			( ((x) >> (y)) & 1UL    )


#define delay(x) _delay_ms(x)
//#define dbg(s) print("dbg: "s)
//#define dbgstr(s) println(s)

// ATtiny led on PB1
#define tiny_led  PB1
#define LED_PIN  tiny_led

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



typedef union {
	uint32_t data;
	struct {
	int16_t integer;
	int16_t decimal;
	};
} float32_t;


uint32_t get_tick(void);

static inline int min(int a, int b) { return a<b ? a : b; }
static inline int max(int a, int b) { return a>b ? a : b; }


#ifdef __cplusplus
}
#endif

#endif // _UTILS_H
