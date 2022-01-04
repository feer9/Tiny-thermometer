/* ds18b20.c - a part of avr-ds18b20 library
 *
 * Copyright (C) 2016 Jacek Wieczorek
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.	See the LICENSE file for details.
 */

/**
	\file
	\brief Implements functions for controlling the DS18B20 sensors
*/

#include <stddef.h>
#include <util/delay.h>
#include <ds18b20/ds18b20.h>
#include <ds18b20/onewire.h>


static uint8_t	*rom = NULL;




void ds18b20_setup(	volatile uint8_t *port, 
					volatile uint8_t *direction, 
					volatile uint8_t *portin, 
					uint8_t mask, 
					uint8_t *_rom )
{
	onewireSetup(port, direction, portin, mask);
	rom = _rom;
}

uint8_t ds18b20_init(	volatile uint8_t *port, 
						volatile uint8_t *direction, 
						volatile uint8_t *portin, 
						uint8_t mask, 
						uint8_t *_rom, 
						uint8_t th, 
						uint8_t tl, 
						uint8_t conf )
{
	ds18b20_setup(port, direction, portin, mask, _rom);

	ds18b20wsp(th, tl, conf);

	float32_t dummy;
	return ds18b20convert_read( &dummy );
}

//! Calculate CRC of provided data
uint8_t ds18b20crc8( uint8_t *data, uint8_t length )
{
	//Generate 8bit CRC for given data (Maxim/Dallas)

	uint8_t i = 0;
	uint8_t j = 0;
	uint8_t mix = 0;
	uint8_t crc = 0;
	uint8_t byte = 0;

	for ( i = 0; i < length; i++ )
	{
		byte = data[i];

		for( j = 0; j < 8; j++ )
		{
			mix = ( crc ^ byte ) & 0x01;
			crc >>= 1;
			if ( mix ) crc ^= 0x8C;
			byte >>= 1;
		}
	}
	return crc;
}

//! Perform ROM matching
void ds18b20match(  )
{
	//Perform ROM match operation on DS18B20 devices
	//Or skip ROM matching if ptr is NULL

	uint8_t i = 0;

	//If rom pointer is NULL then read temperature without matching.
	if ( rom == NULL )
	{
		//Skip ROM
		onewireWrite( DS18B20_COMMAND_SKIP_ROM );
	}
	else
	{
		//Match ROM
		onewireWrite( DS18B20_COMMAND_MATCH_ROM );
		for ( i = 0; i < 8; i++ )
			onewireWrite( rom[i] );
	}
}

//! Request temperature conversion and wait for it's completion, then get the result
uint8_t ds18b20convert_read( float32_t *temperature )
{
	//Send conversion request to DS18B20 on one wire bus

	//Communication check
	if ( onewireInit() == ONEWIRE_ERROR_COMM )
		return DS18B20_ERROR_COMM;

	//ROM match (or not)
	ds18b20match();

	//Convert temperature
	onewireWrite( DS18B20_COMMAND_CONVERT );

	// Wait for convertion to complete
	onewireWait();

	return ds18b20read( temperature );
}

//! Request temperature conversion
uint8_t ds18b20convert(  )
{
	//Send conversion request to DS18B20 on one wire bus

	//Communication check
	if ( onewireInit() == ONEWIRE_ERROR_COMM )
		return DS18B20_ERROR_COMM;

	//ROM match (or not)
	ds18b20match();

	//Convert temperature
	onewireWrite( DS18B20_COMMAND_CONVERT );

	return DS18B20_ERROR_OK;
}

//! Read sensor scratchpad contents
uint8_t ds18b20rsp( uint8_t *sp )
{
	//Read DS18B20 scratchpad

	uint8_t i = 0;

	//Communication check
	if ( onewireInit() == ONEWIRE_ERROR_COMM )
		return DS18B20_ERROR_COMM;

	//Match (or not) ROM
	ds18b20match();

	//Read scratchpad
	onewireWrite( DS18B20_COMMAND_READ_SP );
	for ( i = 0; i < 9; i++ )
		sp[i] = onewireRead();

	//Check pull-up
	if ( ( sp[0] | sp[1] | sp[2] | sp[3] | sp[4] | sp[5] | sp[6] | sp[7] ) == 0 )
		return DS18B20_ERROR_PULL;

	//CRC check
	if ( ds18b20crc8( sp, 8 ) != sp[8] )
		return DS18B20_ERROR_CRC;

	return DS18B20_ERROR_OK;
}

//! Write sensor scratchpad
uint8_t ds18b20wsp( uint8_t th, uint8_t tl, uint8_t conf )
{
	//Writes DS18B20 scratchpad
	//th - thermostat high temperature
	//tl - thermostat low temperature
	//conf - configuration byte

	//Communication check
	if ( onewireInit() == ONEWIRE_ERROR_COMM )
		return DS18B20_ERROR_COMM;

	//ROM match (or not)
	ds18b20match();

	//Write scratchpad
	onewireWrite( DS18B20_COMMAND_WRITE_SP );
	onewireWrite( th );
	onewireWrite( tl );
	onewireWrite( conf );

	return DS18B20_ERROR_OK;
}

//! Copy scratchpad to EEPROM
uint8_t ds18b20csp(  )
{
	//Copies DS18B20 scratchpad contents to its EEPROM

	//Communication check
	if ( onewireInit() == ONEWIRE_ERROR_COMM )
		return DS18B20_ERROR_COMM;

	//ROM match (or not)
	ds18b20match();

	//Copy scratchpad
	onewireWrite( DS18B20_COMMAND_COPY_SP );

	//Set pin high
	//Poor DS18B20 feels better then...
	onewireSetHigh();

	return DS18B20_ERROR_OK;
}

//! Read temperature
uint8_t ds18b20read( float32_t *temperature )
{
	//Read temperature from DS18B20
	//Note: returns actual temperature * 16

	uint8_t sp[9];
	uint8_t ec = 0;

	//Communication, pull-up, CRC checks happen here
	ec = ds18b20rsp( sp );

	if ( ec != DS18B20_ERROR_OK )
	{
		temperature->data = 0;
		return ec;
	}

	//Get temperature from received data
	temperature->integer = (int16_t) ((sp[1] << 8 ) + sp[0]) / 16 ;
	temperature->decimal = (int16_t) (sp[0] & 0x0F) * 10000 / 16 ;

	return DS18B20_ERROR_OK;
}

//! Read ROM address
uint8_t ds18b20rom(  )
{
	//Read DS18B20 rom

	unsigned char i = 0;

	if ( rom == NULL ) return DS18B20_ERROR_OTHER;

	//Communication check
	if ( onewireInit() == ONEWIRE_ERROR_COMM )
		return DS18B20_ERROR_COMM;

	//Read ROM
	onewireWrite( DS18B20_COMMAND_READ_ROM );
	for ( i = 0; i < 8; i++ )
		rom[i] = onewireRead();

	//Pull-up check
	if ( ( rom[0] | rom[1] | rom[2] | rom[3] | rom[4] | rom[5] | rom[6] | rom[7] ) == 0 ) return DS18B20_ERROR_PULL;

	//Check CRC
	if ( ds18b20crc8( rom, 7 ) != rom[7] )
	{
		for ( i = 0; i < 8; i++ ) rom[i] = 0;
		return DS18B20_ERROR_CRC;
	}

	return DS18B20_ERROR_OK;
}

//! Searches for connected sensors
uint8_t ds18b20search( uint8_t *romcnt, uint8_t *roms, uint16_t buflen )
{
	uint8_t bit, currom = 0;
	uint64_t i, junction = 0, rom;
	uint8_t sreg = SREG;

	//romcnt is crucial
	if ( romcnt == NULL ) return DS18B20_ERROR_OTHER;

	#ifdef DS18B20_AUTO_CLI
		cli( );
	#endif

	// 1 loop - 1 thermometer discovered
	do
	{
		// Reset current ROM buffer
		rom = 0;

		// Initiate ROM search
		if ( onewireInit() == ONEWIRE_ERROR_COMM )
		{
			*romcnt = 0;
			SREG = sreg;
			return DS18B20_ERROR_COMM;
		}
		onewireWrite( DS18B20_COMMAND_SEARCH_ROM );

		// Access 64 bits of ROM
		for ( i = 1; i; i <<= 1 )
		{
			//Request two complementary bits from sensors
			bit = 0;
			bit |= onewireReadBit();
			bit |= onewireReadBit() << 1;

			switch ( bit )
			{
				//Received 11 - no sensors connected
				case 3:
					*romcnt = 0; //Null pointer check is at the begining
					SREG = sreg;
					return DS18B20_ERROR_COMM;
					break;

				//Received 10 or 01 - ROM bits match
				case 1:
				case 2:
					bit &= 1;
					break;

				//Received 00 - ROM bits differ
				case 0:
					// Check if there are older junction bits set
					// If there are older bits set, junction value
					// shall be at least equal to i*2
					if ( junction >= ( i << 1 ) )
					// if ( junction >= ( 2 << i ) )
					{
						// Send complement of junction bit
						bit = !( junction & i );
					}
					else
					{
						// Send value of junction bit and toggle it afterwards
						bit = ( junction & i ) != 0;
						junction ^= i;
					}
					break;
			}

			// Send response bit depending on junction status
			// At this point bit is either 0 or 1 and corresponds
			// to the discovered ROM value
			onewireWriteBit( bit );

			// Set bit in temporary ROM buffer
			rom |= bit ? i : 0;
		}

		// Copy prepared ROM to its destination
		if ( roms != NULL && ( currom + 1U ) << 3 <= buflen )
			memcpy( roms + ( currom << 3 ), &rom, 8 );
	}
	while ( ++currom && junction ); // As long as there are junction bits set

	*romcnt = currom;
	SREG = sreg;
	if ( currom == 0 ) return DS18B20_ERROR_COMM; // Exit because of currom overflow (junction broken?)
	return DS18B20_ERROR_OK;
}
