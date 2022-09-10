/**
 * Based on TinuDHT - Library for the DHT11 temperature/humidity sensors
 *
 * @author Neven Boyanov
 * Modified by Fernando Coda
 *
 * TinuDHT is part of the Tinusaur/TinuDHT project.
 *
 * Copyright (c) 2018 Neven Boyanov, The Tinusaur Team. All Rights Reserved.
 * Distributed as open source software under MIT License, see LICENSE.txt file.
 * Retain in your source code the link http://tinusaur.org to the Tinusaur project.
 *
 */

// ----------------------------------------------------------------------------

#include "DHT11.h"
#include "app.h"
#include "utils.h"

#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>


// ----------------------------------------------------------------------------
#define F_MHZ (F_CPU / 1000000)

#define DHT_ACK_TIMEOUT			(10 * F_MHZ)	// @ 1 MHz: 10.. (no less than that)
// NOTE: This should change with the F_CPU change.
#define DHT_BITSTART_TIMEOUT	(6  * F_MHZ)	// @ 1 MHz: 6.. (no less than that)
// NOTE: This should change with the F_CPU change.
#define DHT_BITEND_TIMEOUT		(10  * F_MHZ)	// @ 1 MHz: 9.. (no less than that)
// NOTE: This should change with the F_CPU change.
#define DHT_BITLEN				(5  * F_MHZ)	// @ 1 MHz: 3..7
// NOTE: This should change with the F_CPU change.

// ----------------------------------------------------------------------------

static uint8_t dht_pin;

void dht11_init(uint8_t pin) {
	dht_pin = pin;

	
	// Delay for DHT11 to stabilize (REQUIRED on power on)
	_delay_ms(100);

	// Make a dummy read to start the sensor
	DHT dummy;
	dht11_read(&dummy);

	// Delay for DHT11 to stabilize (REQUIRED on power on)
	_delay_ms(1200);
}

static uint8_t recv(uint8_t rise_timeout, uint8_t fall_timeout)
{
	// idk why tf this doesn't work without the pin being a variable 
	// todo: fix this?

	// Wait for the bit start (LO-to-HI)
	while(pin_isClear(dht_pin) && rise_timeout--);
	
	if (rise_timeout == 0)
		return rise_timeout;

	// Wait for the bit end (HI-to-LO)
	while(pin_isSet(dht_pin) && fall_timeout--);
	
	return fall_timeout;
}

static void send_request(void)
{
	// Send request to DHT11
	pinmode_output(DHT_Pin);

	pin_clear(DHT_Pin);
	_delay_ms(18);

	// Pull to HI and wait for response
	pin_set(DHT_Pin);
	_delay_us(40);
	pinmode_input(DHT_Pin);
}

int8_t dht11_read(DHT *pdht)
{
	send_request();
	
	// Acknowledge from DHT11
	// NOTE: the timeout should be the equivalent of ~ 80 uS
	if(recv(DHT_ACK_TIMEOUT, DHT_ACK_TIMEOUT) == 0)
		return DHT_ERROR_ACK_TIMEOUT;

	uint8_t checksum = 0;

	for (uint8_t nbyte = 0; nbyte < 5; nbyte++)
	{
		uint8_t byte = 0;

		for (int8_t nbit = 7; nbit >= 0; nbit--)
		{
			// NOTE: the timeout should be the equivalent of ~ 50 uS START and 70 us STOP
			uint8_t timeout = recv(DHT_BITSTART_TIMEOUT, DHT_BITEND_TIMEOUT );

			if(timeout == 0)
				return DHT_ERROR_BIT_TIMEOUT;

			// Determine the bit value
			uint8_t len = DHT_BITEND_TIMEOUT - timeout;
			uint8_t bit_value = (len >= DHT_BITLEN);

			byte |= bit_value << nbit;
		}

		if (nbyte < 4)
			checksum += byte;

		pdht->data[nbyte] = byte;
	}

	if (pdht->checksum != checksum)
		return DHT_ERROR_CHECKSUM;
	return DHT_OK;
}
