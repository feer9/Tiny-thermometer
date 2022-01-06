#include "app.h"

static void fsm_thermomether(void);
static void drawRaindrop(void);
void screenOFF(void);

static uint8_t screen_st = SCREEN_OFF;
extern uint32_t last_activity;



void buttonSimpleAction(void)
{
	uint8_t prev_st = screen_st;

	if( (screen_st == SCREEN_OFF) || (++screen_st == SCREEN_MAX) )
		screen_st = SCREEN_DHT11;

	switch(screen_st)
	{
	case SCREEN_DHT11:
	default:
		prepareDisplay_dht11();
		break;
	case SCREEN_DS18B20:
		prepareDisplay_ds18b20();
		break;
	}
	if(prev_st == SCREEN_OFF) {
		ssd1306_on();
	}
}

void buttonLongPressed(void)
{
	if(screen_st != SCREEN_OFF)
	{
		screenOFF();
	}
}

uint8_t getScreenState(void)
{
	return screen_st;
}

void screenLoop(void)
{
	if(screen_st == SCREEN_DHT11) {
		loop_dht11(false);
	}
	else if (screen_st == SCREEN_DS18B20) {
		loop_ds18b20(false);
	}
}


/* ------------------------------------------------------------- */

void screenOFF(void)
{
	screen_st = SCREEN_OFF;
	ssd1306_off();
}

void prepareDisplay_dht11(void)
{
	ssd1306_setFont(ssd1306xled_font8x16);
	ssd1306_clear();
	ssd1306_drawImage( img_thermometer_cold, 0,0);
	ssd1306_printString(20, PAGE1, "Temp:  -");
	ssd1306_printString(20, PAGE3, "Hum.:  -");
	loop_dht11(true);
}

void prepareDisplay_ds18b20(void) 
{
	ssd1306_setFont(CantarellExtraBold_12x16);
	ssd1306_clear();
	drawRaindrop();
	loop_ds18b20(true);
}

static void fsm_thermomether(void)
{
	static uint32_t T = 0UL;
	static uint8_t st = 0;
	const uint32_t curr_tick = get_tick();

	switch (st) {
	case 0:
		if(curr_tick >= T) {
			ssd1306_drawImage( img_thermometer_cold, 0,0 );
			T = curr_tick+1000;
			++st;
		}
		break;
	case 1:
		if(curr_tick >= T) {
			ssd1306_drawImage( img_thermometer_warm, 0,0 );
			T = curr_tick+1000;
			++st;
		}
		break;
	case 2:
	default:
		if(curr_tick >= T) {
			ssd1306_drawImage( img_thermometer_hot, 0,0 );
			T = curr_tick+1000;
			st = 0;
		}
		break;
	}
}

void loop_dht11(bool force_update)
{
	static TinuDHT_data tinudht = { 0 };
	static uint32_t T_dht11 = 0UL;
	const uint32_t curr_tick = get_tick();

	// update dht11
	if(curr_tick > T_dht11 || force_update)
	{
		T_dht11 = curr_tick+1500;

		// Read DHT11 data
		tinudht.status = tinudht_read(&tinudht.current, TINUDHT_PIN);

		if(tinudht.status != TINUDHT_OK)
		{
			const char* buf = _ERR_MSG;
			ssd1306_printString(68, PAGE1, buf);
			ssd1306_printNumberTo(68, PAGE3, tinudht.status);
		}
		else
		{
			if((tinudht.current.t != tinudht.last.t) || force_update)
			{
				float32_t tmp;
				tmp.integer = tinudht.current.temperature;
				tmp.decimal = tinudht.current.temp_dec;
				ssd1306_printFloatTo (68, PAGE1, tmp, 1, 5);
				ssd1306_printString(68+4*8, PAGE1, " " DEG);
			}
			if((tinudht.current.humidity != tinudht.last.humidity) || force_update)
			{
				ssd1306_printNumberTo(68, PAGE3, tinudht.current.humidity);
				ssd1306_printString(68+2*8, PAGE3, "    %");
			}
		}
		tinudht.last = tinudht.current;
	}

	fsm_thermomether();
}

void loop_ds18b20(bool force_update)
{
	static DS18B20_data ds18b20 = { 0 };
	static uint32_t T_ds18b20 = 0UL;
	const uint32_t curr_tick = get_tick();

	// update ds18b20
	if(curr_tick > T_ds18b20 || force_update) 
	{
		T_ds18b20 = curr_tick+500;
		
		// Read ds18b20 data
		ds18b20.status = ds18b20convert_read( &ds18b20.current );
		
		if(ds18b20.status != DS18B20_OK)
		{
			ssd1306_printString(40, PAGE2, _ERR_MSG);
		}
		else if(( ds18b20.last.data != ds18b20.current.data ) || force_update)
		{
			ds18b20.last = ds18b20.current;
			ssd1306_printFloatTo (40, PAGE2, ds18b20.current, 1, 5);
			ssd1306_printString(40+5*ssd1306_getFontWidth(), PAGE2, DEG);
		}
	}
}

static void drawRaindrop(void)
{
	ssd1306_drawImage(img_raindrop, 0,0 );
}
