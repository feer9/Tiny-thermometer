#include "utils.h"
#include <avr/interrupt.h>

static volatile uint32_t tick = 0;

void Timer0_init(void)
{
    // Timer0 interrupt at 1 KHz

    cli();								// Stop interrupts

    GTCCR &= ~(1<<PSR0);				// Clear Prescaler Reset Timer/Counter0
	GTCCR |=  (1<<TSM);					// Set Timer/Counter Synchronization Mode
    TCCR0A = 0;							// Clear Control Register A
    TCNT0 = 0;							// Initialize counter value to 0

#if (F_CPU == 16000000)
	TCCR0B = (1<<CS01) | (1<<CS00);		// PCK / 64
	OCR0A  = 250 - 1;					// Set Output Compare Register for 1 KHz increments
#elif (F_CPU == 8000000)
	TCCR0B = (1<<CS01) | (1<<CS00);		// PCK / 64
	OCR0A  = 125 - 1;					// Set Output Compare Register for 1 KHz increments
#elif (F_CPU == 1000000)
	TCCR0B = (1<<CS01);					// PCK / 8
	OCR0A  = 125 - 1;					// Set Output Compare Register for 1 KHz increments
#else
#error Invalid F_CPU value
#endif
    
    TCCR0A |= (1<<WGM01);				// Turn on CTC mode
    TIMSK |=  (1<<OCIE0A);				// Enable Timer Compare Interrupt
	GTCCR &= ~(1<<TSM);					// Start the Timer/Counter0

	sei();								// Allow interrupts
}


ISR(TIMER0_COMPA_vect)
{
    ++tick;
}

uint32_t get_tick(void) { return tick; }



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
