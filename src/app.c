#include "app.h"

extern uint32_t curr_tick;

void prepareDisplay_dht11(void)
{
  ssd1306_setFont(ssd1306xled_font8x16);
  ssd1306_clear();
  ssd1306_drawImage( img_thermometer_cold, 0,0);
  ssd1306_printString(20, PAGE1, "Temp:  -");
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
    case 2: default:
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

  if(curr_tick > T_dht11 || force_update) { // update dht11
    T_dht11 = curr_tick+1500;
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

  if(curr_tick > T_ds18b20 || force_update) { // update ds18b20
    T_ds18b20 = curr_tick+250;
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

