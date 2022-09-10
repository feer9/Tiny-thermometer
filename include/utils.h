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

/*
#define pin_set(n)    (PORTB |=  (1U << (n)))
#define pin_clear(n)  (PORTB &= ~(1U << (n)))

// note: toggling a pin that is set as an input causes to toggle it's pull-up resistor
#define pin_toggle(n) (PINB |= (1U << (n)))

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
*/

static inline void pin_set  (uint8_t n)  {PORTB |=  (1U << (n));}
static inline void pin_clear(uint8_t n)  {PORTB &= ~(1U << (n));}

// note: toggling a pin that is set as an input causes to toggle it's pull-up resistor
static inline void pin_toggle(uint8_t n) {PINB |= (1U << (n));}

static inline void pinmode_output(uint8_t n) {DDRB |=  (1U << (n));}
static inline void pinmode_input (uint8_t n) {DDRB &= ~(1U << (n));}

// note: pull-up can only be set when the corresponding pin is set as an input
static inline void pinmode_pullup_on (uint8_t n) {PORTB |=  (1U << (n));}
static inline void pinmode_pullup_off(uint8_t n) {PORTB &= ~(1U << (n));}

#define pin_get(n)    ((PINB >> (n)) & 1U)

#define pin_isSet(n)     ((PINB >> (n)) & 1U)
#define pin_isClear(n) (!((PINB >> (n)) & 1U))

//General use bit manipulating macros
#define BitSet(    x, y)			(  (x) |=  (1UL << (y)) )
#define BitClear(  x, y)			(  (x) &= ~(1UL << (y)) )
#define BitToggle( x, y)			(  (x) ^=  (1UL << (y)) )
#define BitCheck(  x, y)			( ((x) >> (y)) & 1UL    )

#define delay(x) _delay_ms(x)
//#define dbg(s) print("dbg: "s)
//#define dbgstr(s) println(s)


typedef union {
	uint32_t data;
	struct {
		int8_t   sign;
		uint8_t  integer;
		uint16_t decimal; // 10^-4
	};
} float32_t;


uint32_t get_tick(void);
void     Timer0_setTickrate(uint16_t);
uint16_t Timer0_getTickrate(void);
void     Timer0_init(void);
void     Tiny_SetClockSpeed(uint8_t freq_MHz);

static inline int min(int a, int b) { return a<b ? a : b; }
static inline int max(int a, int b) { return a>b ? a : b; }


#ifdef __cplusplus
}
#endif

#endif // _UTILS_H
