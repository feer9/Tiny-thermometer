#include "utils.h"
#include <avr/interrupt.h>

static volatile uint32_t tick = 0;

void Timer0_init(void)
{
    // Timer0 interrupt at 1000 Hz (1 ms)

    cli();								// Stop interrupts

    GTCCR &= ~(1<<PSR0);				// Clear Prescaler Reset Timer/Counter0
	GTCCR |=  (1<<TSM);					// Set Timer/Counter Synchronization Mode
    TCCR0A = 0;							// Clear Control Register A
    TCNT0 = 0;							// Initialize counter value to 0


    // Set Prescaler and Output Compare Register for 1 KHz periods
#if (F_CPU == 16000000)
	TCCR0B = (1<<CS01) | (1<<CS00);		// 16000000 / 64 = 250000
	OCR0A  = 250 - 1;					
#elif (F_CPU == 8000000)
	TCCR0B = (1<<CS01) | (1<<CS00);		// 8000000 / 64  = 125000
	OCR0A  = 125 - 1;					
#elif (F_CPU == 1000000)
	TCCR0B = (1<<CS01);					// 1000000 / 8   = 125000
	OCR0A  = 125 - 1;					
#else
#error Invalid F_CPU value
#endif

    TCCR0A |= (1<<WGM01);				// Turn on CTC mode
    TIMSK |=  (1<<OCIE0A);				// Enable Timer Compare Interrupt
	GTCCR &= ~(1<<TSM);					// Start the Timer/Counter0

    // This function doesn't enable the ISR. sei() must be called later on.
}


ISR(TIMER0_COMPA_vect)
{
    ++tick;
}

uint32_t get_tick(void)
{
    return tick;
}



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
