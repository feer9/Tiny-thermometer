/* onewire.c - a part of avr-ds18b20 library
 *
 * Copyright (C) 2016 Jacek Wieczorek
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.	See the LICENSE file for details.
 */

/**
	\file
	\brief Implements 1wire protocol functions
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <inttypes.h>
#include <ds18b20/onewire.h>
#include <stddef.h>

static volatile uint8_t *_1Wport = NULL;
static volatile uint8_t *_1Wdirection = NULL;
static volatile uint8_t *_1Wportin = NULL;
static uint8_t	_1Wmask = 0;


//! Configure 1wire registers
uint8_t onewireSetup( volatile uint8_t *_port, volatile uint8_t *_direction, volatile uint8_t *_portin, uint8_t _mask )
{
	_1Wport = _port;
	_1Wdirection = _direction;
	_1Wportin = _portin;
	_1Wmask = _mask;
	return ONEWIRE_ERROR_OK;
}

//! Initializes 1wire bus before transmission
uint8_t onewireInit()
{
	uint8_t response = 0;
	uint8_t sreg = SREG; //Store status register

	#ifdef ONEWIRE_AUTO_CLI
		cli( );
	#endif

	*_1Wport |= _1Wmask; //Write 1 to output
	*_1Wdirection |= _1Wmask; //Set port to output
	*_1Wport &= ~_1Wmask; //Write 0 to output

	_delay_us( 600 );

	*_1Wdirection &= ~_1Wmask; //Set port to input

	_delay_us( 70 );

	response = *_1Wportin & _1Wmask; //Read input

	_delay_us( 200 );

	*_1Wport |= _1Wmask; //Write 1 to output
	*_1Wdirection |= _1Wmask; //Set port to output

	_delay_us( 600 );

	SREG = sreg; //Restore status register

	return response != 0 ? ONEWIRE_ERROR_COMM : ONEWIRE_ERROR_OK;
}

//! Sends a single bit over the 1wire bus
uint8_t onewireWriteBit( uint8_t bit )
{
	(void) _1Wportin;
	uint8_t sreg = SREG;

	#ifdef ONEWIRE_AUTO_CLI
		cli( );
	#endif

	*_1Wport |= _1Wmask; //Write 1 to output
	*_1Wdirection |= _1Wmask;
	*_1Wport &= ~_1Wmask; //Write 0 to output

	if ( bit != 0 ) _delay_us( 8 );
	else _delay_us( 80 );

	*_1Wport |= _1Wmask;

	if ( bit != 0 ) _delay_us( 80 );
	else _delay_us( 2 );

	SREG = sreg;

	return bit != 0;
}

//! Transmits a byte over 1wire bus
void onewireWrite( uint8_t data )
{
	uint8_t sreg = SREG; //Store status register
	uint8_t i = 0;

	#ifdef ONEWIRE_AUTO_CLI
		cli( );
	#endif

	for ( i = 1; i != 0; i <<= 1 ) //Write byte in 8 single bit writes
		onewireWriteBit( data & i );

	SREG = sreg;
}

//! Reads a bit from the 1wire bus
uint8_t onewireReadBit(  )
{
	uint8_t bit = 0;
	uint8_t sreg = SREG;

	#ifdef ONEWIRE_AUTO_CLI
		cli( );
	#endif

	*_1Wport |= _1Wmask; //Write 1 to output
	*_1Wdirection |= _1Wmask;
	*_1Wport &= ~_1Wmask; //Write 0 to output
	_delay_us( 2 );
	*_1Wdirection &= ~_1Wmask; //Set port to input
	_delay_us( 5 );
	bit = ( ( *_1Wportin & _1Wmask ) != 0 ); //Read input
	_delay_us( 60 );
	SREG = sreg;

	return bit;
}

//! Reads a byte from the 1wire bus
uint8_t onewireRead(  )
{
	uint8_t sreg = SREG; //Store status register
	uint8_t data = 0;
	uint8_t i = 0;

	#ifdef ONEWIRE_AUTO_CLI
		cli( );
	#endif

	for ( i = 1; i != 0; i <<= 1 ) //Read byte in 8 single bit reads
		data |= onewireReadBit() * i;

	SREG = sreg;

	return data;
}

#include <avr/cpufunc.h>
inline void onewireWait(  )
{
	while((*_1Wportin & _1Wmask) == 0)
		_NOP();
}

inline void onewireSetHigh(  )
{
	*_1Wport |= _1Wmask;
	*_1Wdirection |= _1Wmask;
}
