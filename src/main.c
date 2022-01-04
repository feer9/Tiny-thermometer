#include "app.h"


/* Attiny85 PINS:

PB0: MOSI SDA
PB1: MISO
PB2:      SCL
PB3: 
PB4: 
PB5: Reset
*/

// TODO: brown-out detection
// TODO: ensure lowest frequency to reduce consumption

uint32_t curr_tick = 0;


static volatile uint8_t button_st = BUTTON_RELEASED;
static volatile uint32_t last_activity = 0;

typedef enum {SCREEN_OFF, SCREEN_DHT11, SCREEN_DS18B20, SCREEN_MAX} screen_state_t;
static uint8_t screen_st = SCREEN_OFF;


#include <avr/interrupt.h>
ISR(PCINT0_vect)
{
	last_activity = get_tick();
	// cómo se va a quedar en deep sleep hasta que mantenga apretado?
	// si estaba dormido tiene que poner un timeout y volver a dormir
}

void buttonSimpleAction(void)
{
	if(screen_st == SCREEN_OFF)  {
		screen_st = SCREEN_DHT11;
		ssd1306_on();
	}
	else if(++screen_st == SCREEN_MAX) 
		screen_st = SCREEN_DHT11;

	switch(screen_st)
	{
	case SCREEN_DHT11:
	default:
		prepareDisplay_dht11();
		break;
	case SCREEN_DS18B20:
		prepareDisplay_ds18b20();
		break;
	}
}

void buttonLongPressed(void)
{
	if(screen_st != SCREEN_OFF)
	{
		screen_st = SCREEN_OFF;
		ssd1306_off();
	}
}

void loop(void)
{
	// freeze the time in which this loop started
	curr_tick = get_tick();

	readButton();

	if(screen_st == SCREEN_DHT11) {
		loop_dht11(false);
	}
	else if (screen_st == SCREEN_DS18B20) {
		loop_ds18b20(false);
	}


	if(screen_st == SCREEN_OFF && (curr_tick-last_activity) > 5000) {
		// Sleep until an external interrupt
		set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	}
	else {
		// Sleep until a timer interrupt
		set_sleep_mode(SLEEP_MODE_IDLE);
	}

	sleep_mode();

	// todo: avoid the use of 1 ms interrupts to be able to IDLE for more time
	//       maybe 50 ms?

	// todo: put to deep sleep after X minutes of the device running without user interaction.
	//       turn off OLED before going to sleep
}

// Basic debounce function based on an Arduino example
void readButton(void)
{
	static uint32_t T_last_change = 0; // time in which the last state change happened
	static uint32_t elapsed = 0;       // time elapsed since last state change
	static uint8_t last_reading = BUTTON_RELEASED;
	static bool longpressed = false;
	uint8_t curr_reading = pin_get(BUTTON_PIN);

	// If the switch changed, due to noise or pressing:
	if(curr_reading != last_reading) {
		// reset the debouncing timer
		T_last_change = curr_tick;
	}

	elapsed = curr_tick-T_last_change;
	if(elapsed > DEBOUNCE_DELAY) {
		// whatever the reading is at, it's been there for longer than
		// the debounce delay, so take it as the actual current state:

		// if the button state has changed:
		if(curr_reading != button_st) {
			button_st     = curr_reading;
			longpressed   = false;
			last_activity = curr_tick;

			if(curr_reading == BUTTON_RELEASED && elapsed < LONG_PRESS_TIME) {
				buttonSimpleAction();
			}
		}
		else if(curr_reading == BUTTON_PRESSED && !longpressed && elapsed >= LONG_PRESS_TIME) {
			longpressed = true;
			buttonLongPressed();
		}
	}

	last_reading = curr_reading;
}



static void enable_interrupts(void)
{
	// Turn on external interrupts on BUTTON_PIN
	//  BUTTON_PIN = PB1 = PCINT1

	// Turn on Pin Change Interrupt Enable on General Interrupt Mask Register
	GIMSK = (1 << PCIE);
	// Enable Pin Change Interrupt on I/O pin 1
	PCMSK |= (1 << PCINT1);

	// Enable Global Interrupt
	sei();
}

static void GPIO_init(void)
{
//  pinmode_output(LED_PIN);
//  pin_clear(LED_PIN);
	pinmode_input(BUTTON_PIN);
	pinmode_pullup_off(BUTTON_PIN);
}

#if 0
#include <avr/wdt.h>
static void WDT_off(void)
{
	wdt_reset();
	/* Clear WDRF in MCUSR */
	MCUSR = 0x00;
	/* Write logical one to WDCE and WDE */
	WDTCR |= (1<<WDCE) | (1<<WDE);
	/* Turn off WDT */
	WDTCR = 0x00;
}
#endif

static void power_management(void)
{
	// Turn off Analog Comparator
	ACSR = (1 << ACD);
	// Shut down Timer1 and ADC
	PRR  = (1 << PRTIM1) | (1 << PRADC);

	// Turn off Watch Dog Timer
//	WDT_off();
}

/*
void eint_pin_enable(uint8_t pin)
{
	PCMSK |= (1 << pin);
}

void eint_pin_disable(uint8_t pin)
{
	PCMSK &= ~(1 << pin);
}*/

void setup(void)
{
	GPIO_init();

	Timer0_init();

	// Init OLED Display
	ssd1306_init(ssd1306_address);
	ssd1306_setFont(ssd1306xled_font8x16);

	// Init sensors
	tinudht_init();
	ds18b20wsp( &PORTB, &DDRB, &PINB, DS18B20_pinMask, NULL, 100, 0, DS18B20_RES11);

	power_management();

	enable_interrupts();
}

void main (void)
{
  setup();

  for(;;)
    loop();
}

/*void test_ds18b20(void)
{
	float32_t f;

	ssd1306_clear();
	drawRaindrop();
	ssd1306_printString(40+5*ssd1306_getFontWidth(), PAGE2, DEG);

	f.integer = -32;
	f.decimal = 13;
	ssd1306_printFloatTo (40, PAGE2, f, 1, 5);
	_delay_ms(2000);

	f.integer = 45;
	f.decimal = 67;
	ssd1306_printFloatTo (40, PAGE2, f, 1, 5);
	_delay_ms(2000);

	f.integer = -78;
	f.decimal = 90;
	ssd1306_printFloatTo (40, PAGE2, f, 1, 5);
	_delay_ms(2000);

	f.integer = 102;
	f.decimal = 6;
	ssd1306_printFloatTo (40, PAGE2, f, 1, 5);
	_delay_ms(2000);

}*/
