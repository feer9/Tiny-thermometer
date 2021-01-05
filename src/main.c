#include "utils.h"
#include "tinudht.h"
#include <ds18b20/ds18b20.h>
#include "SSD1306_minimal.h"
#include <avr/pgmspace.h>
#include <avr/sleep.h>

#define delay(x) _delay_ms(x)
#define LED_PIN  tiny_led

/* Attiny85 PINS:

PB0: MOSI SDA
PB1: MISO
PB2:      SCL
PB3: 
PB4: 
PB5: Reset
*/

#define DEG "\xa7" "C"

//static SSD1306_Mini oled;
static const uint8_t ds18b20_pinMask = (1 << DS18B20_PIN);

//byte array of bitmap 5x24px
static const unsigned char  img_thermometer_cold[] PROGMEM = {
  0x00, 0xfe, 0x03, 0xfe, 0x50,
  0x00, 0xff, 0x00, 0xff, 0x55,
  0x60, 0x9f, 0x80, 0x9f, 0x65
};

//byte array of bitmap 5x24px
static const unsigned char  img_thermometer_warm[] PROGMEM = {
  0x00, 0xfe, 0x03, 0xfe, 0x50,
  0x00, 0xff, 0x00, 0xff, 0x55,
  0x60, 0xff, 0xff, 0xff, 0x65
};

//byte array of bitmap 5x24px
static const unsigned char  img_thermometer_hot[] PROGMEM = {
  0x00, 0xfe, 0x03, 0xfe, 0x50,
  0x00, 0xff, 0xff, 0xff, 0x55,
  0x60, 0xff, 0xff, 0xff, 0x65
};

void prepareDisplay() {

  ssd1306_clear();
  ssd1306_startScreen();

  ssd1306_drawImage( img_thermometer_cold, 5,1, 5, 3 );

  ssd1306_printStringTo(20, 1, "Water: -");
  ssd1306_printStringTo(20, 2, "Room:  -");
  ssd1306_printStringTo(20, 3, "Hum.:  -");
}

void setup(){
  pinmode_output(LED_PIN);
  pin_clear(LED_PIN);

  ssd1306_init(ssd1306_address);

  prepareDisplay();

	_delay_ms(200);	// Delay for DHT11 to stabilize (REQUIRED on power on)
  ds18b20wsp( &PORTB, &DDRB, &PINB, ds18b20_pinMask, NULL, 100, 0, DS18B20_RES11);

  Timer0_init();

  ACSR = _BV(ACD); // Turn off Analog Comparator
  PRR = _BV(PRTIM1) | _BV(PRADC); // Shut down Timer1 and ADC
}

void loop() {

  static TinuDHT tinudht;
  static TinuDHT tinudht_last;
  static uint8_t tinudht_status;

  static float32_t ds18b20;
  static float32_t ds18b20_last;
  static uint8_t ds18b20_status = DS18B20_OK;
  
  static uint32_t t1=0, t2=0, t3=0;
  uint32_t tick = get_tick();

  if(tick > t1) { // update ds18b20
    t1 = tick+250;
    pin_set(tiny_led); 
    
    // Read ds18b20 data
    ds18b20_status = ds18b20convert_read( &PORTB, &DDRB, &PINB, ds18b20_pinMask, NULL, &ds18b20 );
    
    if(ds18b20_status != DS18B20_OK) {
      ssd1306_printStringTo(68, 1, _ERR_MSG);
    }
    else if( ds18b20_last.data != ds18b20.data )
    {
      ds18b20_last = ds18b20;
      ssd1306_printFloatTo(68,1, ds18b20, 1);
      ssd1306_printStringTo(UNITS_POSITION, 1, DEG);
    }

    pin_clear(tiny_led);
  }

  if(tick > t2) { // update dht11
    t2 = tick+1500;
    pin_set(tiny_led);

    // Read DHT11 data
    tinudht_status = tinudht_read(&tinudht, TINUDHT_PIN);

    if(tinudht_status != TINUDHT_OK)
    {
      const char* buf = _ERR_MSG;
      ssd1306_printStringTo(68, 2, buf);
      ssd1306_printStringTo(68, 3, buf);
    }
    else
    {
      if(tinudht.t != tinudht_last.t)
      {
        float32_t tmp;
        tmp.integer = tinudht.temperature; tmp.decimal = tinudht.temp_dec;
        ssd1306_printFloatTo(68,2, tmp, 1);
        ssd1306_printStringTo(UNITS_POSITION, 2, DEG);
      }
      if(tinudht.humidity != tinudht_last.humidity)
      {
        ssd1306_printNumberTo(68,3, tinudht.humidity);
        ssd1306_printStringTo(UNITS_POSITION + 7, 3, "%");
      }
    }
    tinudht_last = tinudht;

    pin_clear(tiny_led);
  }

  if(tick > t3) { // thermometer animation
    t3 = tick+1000;
    static uint8_t st = 0;
    if(st == 0)
      ssd1306_drawImage( img_thermometer_cold, 5,1, 5, 3 );
    else if (st == 1)
      ssd1306_drawImage( img_thermometer_warm, 5,1, 5, 3 );
    else
      ssd1306_drawImage( img_thermometer_hot,  5,1, 5, 3 );
    st = (st+1)%3;
  }

  // Set sleep mode to IDLE, this allows TIMER/COUNTER0 to keep running
  set_sleep_mode(SLEEP_MODE_IDLE);
  // Sleep until a timer interrupt
  sleep_mode();
}

int main(void)
{
  setup();
  while(true) loop();
}
