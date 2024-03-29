#include "app.h"

static void fsm_thermomether(void);
static void drawRaindrop(void);
void screenOFF(void);

static uint8_t screen_st = SCREEN_DHT11;
static bool force_update = true; // required at startup to fill screen
extern uint32_t last_activity;

// todo: autodetect presence of ds18b20 and enable/disable according
// todo: custom icons depending on temperature level: cold, nice, and hot af


static void prepareDisplay(void)
{
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
}

void appSetup(void)
{
	// Init OLED Display
	ssd1306_init(ssd1306_address);
	ssd1306_setFont(ssd1306xled_font8x16);

	prepareDisplay();

	// Init sensors
	dht11_init(DHT_Pin);
	ds18b20_init(&PORTB, &DDRB, &PINB, DS18B20_pinMask, NULL, 100, 0, DS18B20_RES11);
}

void buttonSimpleAction(void)
{
	uint8_t prev_st = screen_st;

	if(screen_st == SCREEN_OFF) {
		screen_st = SCREEN_DHT11;
	}
	else {
		screen_st++; 
		if(screen_st == SCREEN_MAX)
			screen_st = SCREEN_DHT11;
	}

	prepareDisplay();

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

void appLoop(void)
{
	if(screen_st == SCREEN_DHT11) {
		loop_dht11();
	}
	else if (screen_st == SCREEN_DS18B20) {
		loop_ds18b20();
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
	ssd1306_printString(20, PAGE1, "Temp:");
	ssd1306_printString(20, PAGE3, "Hum.:");
	force_update = true;
}

void prepareDisplay_ds18b20(void) 
{
	ssd1306_setFont(CantarellExtraBold_12x16);
	ssd1306_clear();
	drawRaindrop();
	force_update = true;
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

void loop_dht11(void)
{
	static DHT_data dht11 = { 0 };
	static uint32_t T_dht11 = 0UL;
	const uint32_t curr_tick = get_tick();

	// update dht11
	if(curr_tick > T_dht11 || force_update)
	{
		if(curr_tick > T_dht11) {
			// Read DHT11 data (but don't read it too often)
			dht11.status = dht11_read(&dht11.current);
		}

		if(dht11.status != DHT_OK)
		{
			const char* buf = _ERR_MSG;
			ssd1306_printString(68, PAGE1, buf);
			ssd1306_printNumber(68, PAGE3, dht11.status);
			memset(&dht11.current.data[0], 0xff, sizeof dht11.current.data);
		}
		else
		{
			if((dht11.current.t != dht11.last.t) || force_update)
			{
				float32_t tmp;
				tmp.integer = dht11.current.temperature - 1; // dirty workaround, it seems to have an offset of ~ +1°C
				tmp.decimal = dht11.current.temp_dec;        // maybe I'm getting the value wrong? FIXME
				ssd1306_printFloat (68, PAGE1, tmp, 1, 5);
				ssd1306_printString(68+4*8, PAGE1, " " DEG);
			}
			if((dht11.current.humidity != dht11.last.humidity) || force_update)
			{
				ssd1306_printNumber(68, PAGE3, dht11.current.humidity);
				ssd1306_printString(68+2*8, PAGE3, "    %");
			}
		}
		T_dht11 = curr_tick+1500;
		dht11.last = dht11.current;
	}

	fsm_thermomether();
	force_update = false;
}

void loop_ds18b20(void)
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
			ssd1306_printFloat (40, PAGE2, ds18b20.current, 1, 5);
			ssd1306_printString(40+5*ssd1306_getFontWidth(), PAGE2, DEG);
		}
	}
	force_update = false;
}

static void drawRaindrop(void)
{
	ssd1306_drawImage(img_raindrop, 0,0 );
}