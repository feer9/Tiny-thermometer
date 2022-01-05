#include "utils.h"
#include <avr/interrupt.h>

#define DEFAULT_TICKRATE 8

void Timer0_setTickrate(uint16_t);

static volatile uint32_t tick = 0;

void Timer0_init(void)
{
    // Timer0 interrupt at 1000 Hz (1 ms)

    cli();								// Stop interrupts

    GTCCR &= ~(1<<PSR0);				// Clear Prescaler Reset Timer/Counter0
	GTCCR |=  (1<<TSM);					// Set Timer/Counter Synchronization Mode
    TCCR0A = 0;							// Clear Control Register A
    TCNT0 = 0;							// Initialize counter value to 0


    // Set Prescaler and Output Compare Register for 1 ms interruptions
	Timer0_setTickrate(DEFAULT_TICKRATE);


    TCCR0A |= (1<<WGM01);				// Turn on CTC mode
    TIMSK |=  (1<<OCIE0A);				// Enable Timer Compare Interrupt
	GTCCR &= ~(1<<TSM);					// Start the Timer/Counter0

    // This function doesn't enable the ISR. sei() must be called later on.
}

static uint16_t tickrate = DEFAULT_TICKRATE;

// T: Time in ms between interruptions
void Timer0_setTickrate(uint16_t T)
{
	tickrate = T;
    switch(T)
    {
        case 50: // 20Hz - 50 ms interrupts
        
#if (F_CPU == 16000000)
			while(1); // can't do 20Hz with 16MHz
#elif (F_CPU == 8000000)
			while(1); // can't do 20Hz with 8MHz
#elif (F_CPU == 1000000)
			TCCR0B = (1<<CS02);					//  1000000 /  256 = 3096.25
			OCR0A  = 195 - 1;					//  3096.25 /  195 =   20.03 (approximated)
#else
	#error Unsupported F_CPU value
#endif      

            break;

        case 8: // 125Hz - 8 ms interrupts
        
#if (F_CPU == 16000000)
            TCCR0B = (1<<CS02) | (1<<CS00);		// 16000000 / 1024 = 15625
            OCR0A  = 125 - 1;					//    15625 /  125 =   125
#elif (F_CPU == 8000000)
            TCCR0B = (1<<CS02);					//  8000000 /  256 = 31250
            OCR0A  = 250 - 1;					//    31250 /  250 =   125
#elif (F_CPU == 1000000)
            TCCR0B = (1<<CS01) | (1<<CS00);		//  1000000 /   64 = 15625
            OCR0A  = 125 - 1;					//    15625 /  125 =   125
#else
	#error Unsupported F_CPU value
#endif      

            break;
        
        case 1: // 1kHz - 1ms interrupts
        default:

#if (F_CPU == 16000000)
            TCCR0B = (1<<CS01) | (1<<CS00);		// 16000000 /  64 = 250000
            OCR0A  = 250 - 1;					//   250000 / 250 =   1000
#elif (F_CPU == 8000000)
            TCCR0B = (1<<CS01) | (1<<CS00);		//  8000000 /  64 = 125000
            OCR0A  = 125 - 1;					//   125000 / 125 =   1000
#elif (F_CPU == 1000000)
            TCCR0B = (1<<CS01);					//  1000000 /   8 = 125000
            OCR0A  = 125 - 1;					//   125000 / 125 =   1000
#else
	#error Unsupported F_CPU value
#endif
            break;

    }
}

/*uint16_t Timer0_getTickrate(void)
{
	return tickrate;
}*/

ISR(TIMER0_COMPA_vect)
{
    tick += tickrate;
}

uint32_t get_tick(void)
{
    return tick;
}



// Set the System Clock Prescaler. 
// Only works with 8MHz internal clock as source.
/*void Tiny_SetClockSpeed(uint8_t freq_MHz)
{
	cli();

	switch (freq_MHz)
	{
		case 8:
			CLKPR = 1 << CLKPCE;
			CLKPR = (1<<CLKPS0) | (1<<CLKPS1);
			g_FCPU = 8000000;
			break;

		case 1:
		default:
			CLKPR = 1 << CLKPCE;
			CLKPR = 0;
			g_FCPU = 1000000;
			break;

		// Didn't bother to implement other frequencies
	}

	sei();
}*/

/*
    // Set Timer1 interrupt at 1 KHz (doesn't work!!!!!!!)

    GTCCR &= ~(0x7E);		// Set General Control Register bits 6:1 to 0
    TCCR1 = 0;				// Set Control Register to 0
    TCNT1 = 0;				// Initialize counter value to 0

	// Set prescaler to get 1 count every 1/125000 seconds
#if (F_CPU == 16000000)
	TCCR1 |= 0x08;			// PCK / 128
#elif (F_CPU == 8000000)
	TCCR1 |= 0x07;			// PCK / 64
#elif (F_CPU == 1000000)
	TCCR1 |= 0x04;			// PCK / 8
#else
#error Invalid F_CPU value
#endif
    // Set compare match register for 1 KHz increments
    OCR1A = 125 - 1;
	// Turn on CTC mode
    TCCR1 |= (1 << CTC1);
    // Enable timer compare interrupt
    TIMSK |= (1 << OCIE1A);
*/
