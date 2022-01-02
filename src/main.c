#include "app.h"


/* Attiny85 PINS:

PB0: MOSI SDA
PB1: MISO
PB2:      SCL
PB3: 
PB4: 
PB5: Reset
*/

// TODO: brown-out detection
// TODO: ensure lowest frequency to reduce consumption

static uint32_t tick = 0;


void prepareDisplay_dht11(void)
{
  ssd1306_setFont(ssd1306xled_font8x16);
  ssd1306_clear();
  ssd1306_drawImage( img_thermometer_cold, 0,0);
  ssd1306_printString(20, PAGE1, "Room:  -");
  ssd1306_printString(20, PAGE3, "Hum.:  -");
  loop_dht11(1);
}

void prepareDisplay_ds18b20(void) 
{
  ssd1306_setFont(CantarellExtraBold_12x16);
  ssd1306_clear();
  drawRaindrop();
  loop_ds18b20(1);
}

void fsm_thermomether(void)
{
  static uint32_t T = 0UL;
  static uint8_t st = 0;


  switch (st) {
    case 0:
      if(tick >= T) {
        ssd1306_drawImage( img_thermometer_cold, 0,0 );
        T = tick+1000;
        ++st;
      }
      break;
    case 1:
      if(tick >= T) {
        ssd1306_drawImage( img_thermometer_warm, 0,0 );
        T = tick+1000;
        ++st;
      }
      break;
    case 2: default:
      if(tick >= T) {
        ssd1306_drawImage( img_thermometer_hot, 0,0 );
        T = tick+1000;
        st = 0;
      }
      break;
  }
}

void loop_dht11(bool force_update)
{
  static TinuDHT_data tinudht = { 0 };
  static uint32_t T_dht11 = 0UL;

  if(tick > T_dht11 || force_update) { // update dht11
    T_dht11 = tick+1500;
//    pin_set(tiny_led);

    // Read DHT11 data
    tinudht.status = tinudht_read(&tinudht.current, TINUDHT_PIN);

    if(tinudht.status != TINUDHT_OK)
    {
      const char* buf = _ERR_MSG;
      ssd1306_printString(68, PAGE1, buf);
      ssd1306_printString(68, PAGE3, buf);
    }
    else
    {
      if((tinudht.current.t != tinudht.last.t) || force_update)
      {
        float32_t tmp;
        tmp.integer = tinudht.current.temperature;
        tmp.decimal = tinudht.current.temp_dec;
        ssd1306_printFloatTo (68, PAGE1, tmp, 1, 5);
        ssd1306_printString(68+5*8, PAGE1, DEG);
      }
      if((tinudht.current.humidity != tinudht.last.humidity) || force_update)
      {
        ssd1306_printString(68, PAGE3, "      %");
        ssd1306_printNumberTo(68, PAGE3, tinudht.current.humidity);
      }
    }
    tinudht.last = tinudht.current;

//    pin_clear(tiny_led);
  }

  fsm_thermomether();
}

void loop_ds18b20(bool force_update)
{
  static DS18B20_data ds18b20 = { 0 };
  static uint32_t T_ds18b20 = 0UL;

  if(tick > T_ds18b20 || force_update) { // update ds18b20
    T_ds18b20 = tick+250;
//    pin_set(tiny_led); 
    
    // Read ds18b20 data
    ds18b20.status = ds18b20convert_read( &PORTB, &DDRB, &PINB, DS18B20_pinMask, NULL, &ds18b20.current );
    
    if(ds18b20.status != DS18B20_OK) {
      ssd1306_printString(40, PAGE2, _ERR_MSG);
    }
    else if(( ds18b20.last.data != ds18b20.current.data ) || force_update)
    {
      ds18b20.last = ds18b20.current;
      ssd1306_printFloatTo (40, PAGE2, ds18b20.current, 1, 5);
      ssd1306_printString(40+5*ssd1306_getFontWidth(), PAGE2, DEG);
    }

//    pin_clear(tiny_led);
  }
}

void drawRaindrop(void)
{
  ssd1306_drawImage(img_raindrop, 0,0 );
}

void setup(void)
{
//  pinmode_output(LED_PIN);
//  pin_clear(LED_PIN);
  pinmode_input(BUTTON_PIN);
  pinmode_pullup_off(BUTTON_PIN);

  // Init OLED Display
  ssd1306_init(ssd1306_address);

  // Init sensors
	_delay_ms(200);	// Delay for DHT11 to stabilize (REQUIRED on power on)
  ds18b20wsp( &PORTB, &DDRB, &PINB, DS18B20_pinMask, NULL, 100, 0, DS18B20_RES11);

  // Init timer
  Timer0_init();

  // Power management
  ACSR = (1 << ACD);                   // Turn off Analog Comparator
  PRR  = (1 << PRTIM1) | (1 << PRADC); // Shut down Timer1 and ADC

  // Turn on external interrupts on BUTTON_PIN
  //  BUTTON_PIN = PB1 = PCINT1
  GIMSK = (1 << PCIE);    // Turn on Pin Change Interrupt Enable on General Interrupt Mask Register
  PCMSK |= (1 << PCINT1); // Enable Pin Change Interrupt on I/O pin 1

  sei(); // Enable Global Interrupt
}

ISR(PCINT0_vect)
{

}


static volatile uint8_t button_st = BUTTON_RELEASED;
static uint32_t activity = 0;

typedef enum {SCREEN_OFF, SCREEN_DHT11, SCREEN_DS18B20, SCREEN_MAX};
static uint8_t screen_st = SCREEN_OFF;

void buttonSimpleAction(void)
{
  if(++screen_st == SCREEN_MAX) screen_st = SCREEN_DHT11;

  switch(screen_st) {
  case SCREEN_DHT11: default:
      prepareDisplay_dht11();
      break;
  case SCREEN_DS18B20:
      prepareDisplay_ds18b20();
      break;
  }
}

void buttonLongPressed(void)
{
  if(screen_st == SCREEN_OFF) {
    screen_st = SCREEN_DHT11;
    prepareDisplay_dht11();
    ssd1306_on();
  }
  else {
    screen_st = SCREEN_OFF;
    ssd1306_off();
/*  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // todo: TEST EXTERNAL INTERRUPT FIRST
    // Sleep until a timer interrupt
    sleep_mode(); */
  }
}

// Basic debounce function based on an Arduino example
void readButton(void)
{
  static uint32_t debounce = 0;     // time in which the last state change happened
  static uint32_t elapsed = 0;      // time elapsed since last state change
  static uint8_t last = 1U;
  static bool longpressed = false;
  uint8_t val = pin_get(BUTTON_PIN);

  // If the switch changed, due to noise or pressing:
  if(val != last) {
    // reset the debouncing timer
    debounce = tick;
  }

  elapsed = tick-debounce;
  if(elapsed > DEBOUNCE_DELAY) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if(val != button_st) {
      button_st   = val;
      longpressed = false;
      activity    = tick;

      if(val == BUTTON_RELEASED && elapsed < LONG_PRESS_TIME) {
        buttonSimpleAction();
      }
    }
    else if(val == BUTTON_PRESSED && !longpressed && elapsed >= LONG_PRESS_TIME) {
      longpressed = true;
      buttonLongPressed();
    }
  }

  last = val;
}

void loop(void)
{
  tick = get_tick();

  readButton();

  if(screen_st == SCREEN_DHT11) {
    loop_dht11(0);
  }
  else if (screen_st == SCREEN_DS18B20) {
    loop_ds18b20(0);
  }
  // Set sleep mode to IDLE, this allows TIMER/COUNTER0 to keep running
  set_sleep_mode(SLEEP_MODE_IDLE);
  // Sleep until a timer interrupt
  sleep_mode();
  // todo: avoid the use of 1 ms interrupts to be able to IDLE for more time
  //       maybe 50 ms?

  // todo: put to deep sleep after X minutes of the device running without user interaction.
  //       turn off OLED before going to sleep
}

void main (void)
{
  setup();

  for(;;)
    loop();
}

void test_ds18b20(void)
{
    float32_t f;

    ssd1306_clear();
    drawRaindrop();
    ssd1306_printString(40+5*ssd1306_getFontWidth(), PAGE2, DEG);

    f.integer = -32;
    f.decimal = 13;
    ssd1306_printFloatTo (40, PAGE2, f, 1, 5);
    _delay_ms(2000);

    f.integer = 45;
    f.decimal = 67;
    ssd1306_printFloatTo (40, PAGE2, f, 1, 5);
    _delay_ms(2000);

    f.integer = -78;
    f.decimal = 90;
    ssd1306_printFloatTo (40, PAGE2, f, 1, 5);
    _delay_ms(2000);

    f.integer = 102;
    f.decimal = 6;
    ssd1306_printFloatTo (40, PAGE2, f, 1, 5);
    _delay_ms(2000);

}

#include <avr/wdt.h>
void WDT_off(void)
{
  wdt_reset();
  /* Clear WDRF in MCUSR */
  MCUSR = 0x00;
  /* Write logical one to WDCE and WDE */
  WDTCR |= (1<<WDCE) | (1<<WDE);
  /* Turn off WDT */
  WDTCR = 0x00;
}
