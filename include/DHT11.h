#ifndef TINUDHT_H
#define TINUDHT_H

// ----------------------------------------------------------------------------

#include <stdint.h>

// ----------------------------------------------------------------------------

#define DHT_OK					0
#define DHT_ERROR_ACK_TIMEOUT	-1
#define DHT_ERROR_BIT_TIMEOUT	-2
#define DHT_ERROR_CHECKSUM		-3

// ----------------------------------------------------------------------------


#ifdef __cplusplus
extern "C" {
#endif

typedef union {
	uint8_t data[5];
	// DHT11 has a range of 0-50°C and 20-90% H so no negative numbers
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
} DHT;

// ----------------------------------------------------------------------------

void dht11_init(uint8_t pin);
int8_t dht11_read(DHT *ptinudht);

// ----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
