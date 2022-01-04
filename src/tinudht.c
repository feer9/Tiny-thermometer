/**
 * TinuDHT - Library for the DHT11 temperature/humidity sensors
 *
 * @author Neven Boyanov
 *
 * This is part of the Tinusaur/TinuDHT project.
 *
 * Copyright (c) 2018 Neven Boyanov, The Tinusaur Team. All Rights Reserved.
 * Distributed as open source software under MIT License, see LICENSE.txt file.
 * Retain in your source code the link http://tinusaur.org to the Tinusaur project.
 *
 * Source code available at: https://bitbucket.org/tinusaur/tinudht
 *
 */

// ----------------------------------------------------------------------------

#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>

#include "tinudht.h"

// ----------------------------------------------------------------------------
#define F_MHZ (F_CPU / 1000000)

#define TINUDHT_ACK_TIMEOUT			(11 * F_MHZ)	// @ 1 MHz: 10.. (no less than that)
// NOTE: This should change with the F_CPU change.
#define TINUDHT_BITSTART_TIMEOUT	(7  * F_MHZ)	// @ 1 MHz: 6.. (no less than that)
// NOTE: This should change with the F_CPU change.
#define TINUDHT_BITEND_TIMEOUT		(10 * F_MHZ)	// @ 1 MHz: 9.. (no less than that)
// NOTE: This should change with the F_CPU change.
#define TINUDHT_BITLEN				(5  * F_MHZ)	// @ 1 MHz: 3..7
// NOTE: This should change with the F_CPU change.

// ----------------------------------------------------------------------------

void tinudht_init(void) {
	// Delay for DHT11 to stabilize (REQUIRED on power on)
	_delay_ms(200);
}

uint8_t tinudht_read(TinuDHT *ptinudht, uint8_t dht_pin) {
	uint8_t timeout;

	memset(ptinudht->data, 0xff, sizeof ptinudht->data);

	// Send request to DHT11
	DDRB |= (1 << dht_pin);	// Set port as output
	PORTB &= ~(1 << dht_pin);	// Set line to LO
	_delay_ms(18);	// Keep line to LO for 18 mS
	// Pull to HI and wait for response
	PORTB |= (1 << dht_pin);	// Set to 1, internal pull-up. (optional)
	_delay_us(20);	// Wait 20 uS for response

	// Setup pin as input, pulls to HI and wait for response
	DDRB &= ~(1 << dht_pin);	// Set port as input
	PORTB |= (1 << dht_pin);	// Set to 1, internal pull-up. (optional)

	// Acknowledge from DHT11
	// NOTE: the timeout should be the equivalent of ~ 80 uS
	timeout = TINUDHT_ACK_TIMEOUT;
	while(bit_is_clear(PINB, dht_pin))	// Wait for LO-to-HI
		if (--timeout == 0)
			return TINUDHT_ERROR_ACK_TIMEOUT;
	timeout = TINUDHT_ACK_TIMEOUT;
	while(bit_is_set(PINB, dht_pin))	// Wait for HI-to-LO
		if (--timeout == 0)
			return TINUDHT_ERROR_ACK_TIMEOUT;

	uint8_t bit_index = 7;
	uint8_t data_index = 0;
	uint8_t data_byte = 0;
	uint8_t checksum = 0;	// NOTE: Should change to 16-bit.
	// Read the data - 40 bits (5 bytes),
	for (uint8_t i = 0; i < 40; i++) {
		// Wait for the bit start
		// NOTE: the timeout should be the equivalent of ~ 50 uS
		timeout = TINUDHT_BITSTART_TIMEOUT;
		while(bit_is_clear(PINB, dht_pin))	// Wait for LO-to-HI
			if (--timeout == 0)
				return TINUDHT_ERROR_BIT_TIMEOUT;
		// Wait for the bit end
		// NOTE: the timeout should be the equivalent of ~ 70 uS
		timeout = TINUDHT_BITEND_TIMEOUT;
		while(bit_is_set(PINB, dht_pin))	// Wait for HI-to-LO
			if (--timeout == 0)
				return TINUDHT_ERROR_BIT_TIMEOUT;
		// Determine the bit value
		uint8_t len = TINUDHT_BITEND_TIMEOUT - timeout;
		data_byte = (data_byte << 1); // Shift left to make space for the next bit.
		if (len >= TINUDHT_BITLEN) data_byte |= 1; // Add a "1" at the end. Otherwise it will remain a "0".
		if (bit_index == 0) {	// Next byte?
			ptinudht->data[data_index] = data_byte;	// Store next byte to the data array.
			if (data_index < 4) checksum += data_byte;	// Add next byte to the checksum.
			data_index++;	// Next byte.
			bit_index = 7;	// Restart at MSB
			data_byte = 0;	// Clear
		} else bit_index--;
	}
	// checksum &= 0xff; // NOTE: Uncomment this after changing checksum to 16-bit.
	// NOTE: On DHT11 data[1],data[3] are always zero so not used.
	if (ptinudht->checksum != checksum) return TINUDHT_ERROR_CHECKSUM;
	return TINUDHT_OK;
}
