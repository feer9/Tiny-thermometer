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

#ifndef TINUDHT_H
#define TINUDHT_H

// ----------------------------------------------------------------------------

#include <stdint.h>

// ----------------------------------------------------------------------------

#define TINUDHT_OK					0
#define TINUDHT_ERROR_ACK_TIMEOUT	-1
#define TINUDHT_ERROR_BIT_TIMEOUT	-2
#define TINUDHT_ERROR_CHECKSUM		-3

// ----------------------------------------------------------------------------


#ifdef __cplusplus
extern "C" {
#endif

typedef union {
	uint8_t data[5];
	struct {
		uint8_t humidity;
		uint8_t hum_dec;
		uint8_t temperature;
		uint8_t temp_dec;
		uint8_t checksum;
	};
	struct {
		uint16_t h;
		uint16_t t;
		uint8_t c;
	};
} TinuDHT;

// ----------------------------------------------------------------------------

void tinudht_init(void);
uint8_t tinudht_read(TinuDHT *ptinudht, uint8_t dht_pin);

// ----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
