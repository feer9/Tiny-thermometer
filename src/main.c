#include "app.h"

static void readButton(void);
static void endLoop(void);

/* Attiny85 PINS:

PB0: MOSI SDA
PB1: MISO
PB2:      SCL
PB3: 
PB4: 
PB5: Reset
*/

// TODO: ensure lowest frequency to reduce consumption
//       can't get too low since the display doesn't appear to like it

static uint32_t last_activity = 0;


void loop(void)
{
	readButton();

	screenLoop();

	endLoop();
}


static void endLoop(void)
{
	uint32_t T_inactive = (get_tick()-last_activity);

	if(T_inactive > LONG_PRESS_TIME)
	{
		uint8_t sleepmode;
		if(getScreenState() == SCREEN_OFF && T_inactive > 5000) {
			// Sleep until an external interrupt
			sleepmode = SLEEP_MODE_PWR_DOWN;
		}
		else if(T_inactive > 60000) {
			// Turn screen OFF and sleep until an external interrupt
			screenOFF();
			sleepmode = SLEEP_MODE_PWR_DOWN;
		}
		else {
			// Sleep until a timer interrupt
			sleepmode = SLEEP_MODE_IDLE;
		}
		
		cli();
		set_sleep_mode(sleepmode);
		sleep_enable();
		if (sleepmode == SLEEP_MODE_PWR_DOWN)
			sleep_bod_disable();
		sei();
		sleep_cpu();
		sleep_disable();
	}
}

// Basic debounce function based on an Arduino example
static void readButton(void)
{
	static uint8_t button_st = BUTTON_RELEASED;
	static uint8_t last_reading = BUTTON_RELEASED;
	static uint32_t T_last_change = 0; // time in which the last state change happened
	static bool longpressed = false;
	uint32_t elapsed;                  // time elapsed since last state change
	uint8_t curr_reading = pin_get(BUTTON_PIN);
	const uint32_t curr_tick = get_tick();

	// If the switch changed, due to noise or pressing:
	if(curr_reading != last_reading) {
		// reset the debouncing timer
		T_last_change = curr_tick;
	}

	elapsed = curr_tick-T_last_change;
	if(elapsed > DEBOUNCE_DELAY)
	{
		// whatever the reading is at, it's been there for longer than
		// the debounce delay, so take it as the actual current state:

		// if the button state has changed:
		if(curr_reading != button_st)
		{	
			button_st = curr_reading;
			last_activity = curr_tick;

			if(curr_reading == BUTTON_RELEASED && elapsed < LONG_PRESS_TIME && !longpressed) {
				buttonSimpleAction();
			}

			longpressed   = false;
		}
		else if(curr_reading == BUTTON_PRESSED && !longpressed && elapsed >= LONG_PRESS_TIME)
		{
			longpressed = true;
			buttonLongPressed();
		}
	}

	last_reading = curr_reading;
}

ISR(PCINT0_vect)
{
	last_activity = get_tick();
}

static void enable_interrupts(void)
{
	// Turn on external interrupts on BUTTON_PIN
	//  BUTTON_PIN = PB1 = PCINT1

	// Set Pin Change Interrupt Enable on General Interrupt Mask Register
	GIMSK = (1 << PCIE);
	// Enable Pin Change Interrupt on PB1
	PCMSK |= (1 << PCINT1);

	// Enable Global Interrupt
	sei();
}

static void GPIO_init(void)
{
	pinmode_input(BUTTON_PIN);
	pinmode_pullup_off(BUTTON_PIN);
}

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


static void power_management(void)
{
	// Turn off Analog Comparator
	ACSR = (1 << ACD);
	// Shut down Timer1 and ADC
	PRR  = (1 << PRTIM1) | (1 << PRADC);

	// Turn off Watch Dog Timer
	WDT_off();
}

void setup(void)
{
	GPIO_init();

	// Stop interrupts
    cli();

	Timer0_init();

	// Init OLED Display
	ssd1306_init(ssd1306_address);
	ssd1306_setFont(ssd1306xled_font8x16);

	// Init sensors
	tinudht_init(TINUDHT_PIN);
	ds18b20_init(&PORTB, &DDRB, &PINB, DS18B20_pinMask, NULL, 100, 0, DS18B20_RES11);

	power_management();

	enable_interrupts();
}

void main (void)
{
  setup();

  for(;;)
    loop();
}
