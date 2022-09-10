/*

  SSD1306_minimal.h - SSD1306 OLED Driver Library
  2015 Copyright (c) CoPiino Electronics All right reserved.
  
  Original Author: GOF Electronics Co. Ltd.
  Modified by: CoPiino Electronics ( http://copiino.cc ) 
  
  CoPiino Electronics invests time and resources providing this open source code, 
	please support CoPiino Electronics and open-source hardware by purchasing 
	products from CoPiino Electronics!
  
  What is it?	
    This library is derived from GOFi2cOLED library, only for SSD1306 in I2C Mode.
    As the original library only supports Frame Buffered mode which requires to have
    at least 1024bytes of free RAM for a 128x64px display it is too big for smaller devices.
    
    So this a SSD1306 library that works great with ATTiny85 devices :)
    
  
  It is a free software; you can redistribute it and/or modify it 
  under the terms of BSD license, check license.txt for more information.
	All text above must be included in any redistribution.
*/

#include "SSD1306_minimal.h"


static uint8_t SlaveAddress = 0;
static fontSettings currentFont = { 0 };

//// Private Functions
static void sendCommand(uint8_t command);
static void sendCommand2(uint8_t command1, uint8_t command2);
/*static*/ void sendData(const uint8_t *data, uint8_t len);
static void sendFlashData(const uint8_t *data, uint16_t len);

static uint8_t getFlash( const unsigned char * mem, unsigned int idx  );
//// End Private Functions


void ssd1306_setFont(const uint8_t *_f) {
  const font_t *f = (font_t *) _f;
  currentFont.data = &f->data;
  currentFont.width      = pgm_read_byte( &(f->width) );
  currentFont.height     = pgm_read_byte( &(f->height) );
  currentFont.first_char = pgm_read_byte( &(f->first_char) );
  currentFont.charSize = ((currentFont.width * currentFont.height) >> 3) +
                        (((currentFont.width | currentFont.height) & 0x07) != 0) ;
}

uint8_t getFlash( const unsigned char * mem, unsigned int idx ) {
  unsigned char data= pgm_read_byte( &(mem[idx]) );
  return data;
}

void sendCommand(unsigned char command) {

  USI_TWI_beginTransmission(SlaveAddress); // begin I2C communication

  USI_TWI_send(GOFi2cOLED_Command_Mode);   // Set OLED Command mode
  USI_TWI_send(command);

  USI_TWI_endTransmission();               // End I2C communication
}

void sendCommand2(uint8_t command1, uint8_t command2) {

  USI_TWI_beginTransmission(SlaveAddress); // begin I2C communication

  USI_TWI_send(GOFi2cOLED_Command_Mode);   // Set OLED Command mode
  USI_TWI_send(command1);
  USI_TWI_send(GOFi2cOLED_Command_Mode);   // Set OLED Command mode
  USI_TWI_send(command2);

  USI_TWI_endTransmission();               // End I2C communication
}

void sendData(const uint8_t *data, uint8_t len) {

  for (uint16_t i=0; i<len; ++i)
  {
    if ( (i % (USI_BUF_SIZE-1)) == 0)
    {
      USI_TWI_endTransmission();

      USI_TWI_beginTransmission(SlaveAddress);
      USI_TWI_send(GOFi2cOLED_Data_Mode);
    }

    USI_TWI_send(*data++);
  }

  USI_TWI_endTransmission();
}

void sendFlashData(const uint8_t *data, uint16_t len) {

  for (uint16_t i=0; i < len; ++i)
  {
    uint8_t byte= getFlash( data, i);
    
    if ( (i % (USI_BUF_SIZE-1)) == 0)
    {
      USI_TWI_endTransmission();

      USI_TWI_beginTransmission(SlaveAddress);
      USI_TWI_send(GOFi2cOLED_Data_Mode);
    }

    USI_TWI_send( byte );  
  }
  
  USI_TWI_endTransmission();  
}

inline void ssd1306_off(void) {
  sendCommand(GOFi2cOLED_Display_Off_Cmd);
}

inline void ssd1306_on(void) {
  sendCommand(GOFi2cOLED_Display_On_Cmd);
}

void ssd1306_init(uint8_t address) {

  SlaveAddress = address;

  USI_TWI_begin();

  _delay_ms(5);	//wait for OLED hardware init

  sendCommand(GOFi2cOLED_Display_Off_Cmd);                       /*display off*/

  sendCommand2(Set_Multiplex_Ratio_Cmd, 31);                     /*multiplex ratio*/

  sendCommand2(Set_Display_Offset_Cmd, 0x00);                    /*set display offset*/

  sendCommand2(Set_Memory_Addressing_Mode_Cmd, HORIZONTAL_MODE); /*set horizontal addressing mode*/

  sendCommand(0xB0 | 0x00);                                      /*set page address*/
  sendCommand(0x00 | 0x00);                                      /*set column lower-nibble address*/
  sendCommand(0x10 | 0x00);                                      /*set column higher-nibble address*/

  sendCommand(0x40);                                             /*set display start line*/

  sendCommand2(Set_Contrast_Cmd, 60);                            /*screen brightness: 1 to 255*/

  sendCommand(Segment_Remap_Cmd);                                /*set segment remap*/

  sendCommand(COM_Output_Remap_Scan_Cmd);                        /*Com scan direction*/

  sendCommand(GOFi2cOLED_Normal_Display_Cmd);                    /*normal / reverse*/

  sendCommand2(Set_Display_Clock_Divide_Ratio_Cmd, 0x80);        /*set osc division*/

  sendCommand2(Set_Precharge_Period_Cmd, 0xf1);                  /*set pre-charge period*/

  sendCommand2(Set_COM_Pins_Hardware_Config_Cmd, 0x02);          /*set COM pins*/

  sendCommand2(Set_VCOMH_Deselect_Level_Cmd, 0x30);              /*set vcomh*/

  sendCommand(Deactivate_Scroll_Cmd);

  sendCommand2(Charge_Pump_Setting_Cmd, Charge_Pump_Enable_Cmd); /*set charge pump enable*/

  ssd1306_clear();

  sendCommand(GOFi2cOLED_Display_On_Cmd);                        /*display ON*/
}

// cmd: Set_Column_Address_Cmd or Set_Page_Address_Cmd
static void ssd1306_clipAxis(uint8_t cmd, uint8_t start, uint8_t end) {
  USI_TWI_begin();                         // initialize I2C
  USI_TWI_beginTransmission(SlaveAddress); // begin I2C transmission
  USI_TWI_send(GOFi2cOLED_Command_Mode);   // command mode
  USI_TWI_send(cmd);
  USI_TWI_send(0);

  USI_TWI_send(start);
  USI_TWI_send(end);

  USI_TWI_endTransmission();               // stop I2C transmission
}

void ssd1306_clipArea(unsigned char col, unsigned char page, unsigned char w, unsigned char h) {
  ssd1306_clipAxis(Set_Column_Address_Cmd, col, col+w-1);
  ssd1306_clipAxis(Set_Page_Address_Cmd, page, page+h-1);
}

void ssd1306_cursorTo(unsigned char col, unsigned char page) {
  ssd1306_clipArea(col, page, 128-col, 4-page); // clip from the point (col,row) to the end
}

void ssd1306_startScreen() {
  sendCommand(0x00 | 0x0); // low col = 0
  sendCommand(0x10 | 0x0); // hi col = 0
  sendCommand(0x40 | 0x0); // line #0
}

void ssd1306_clear() {
  
  ssd1306_startScreen();
  ssd1306_clipArea(0,0,128,4);
  
  for (uint16_t i=0; i<=((128*32/8)/16); i++) // package size = 16
  {
    // send a bunch of data in one xmission
    USI_TWI_beginTransmission(SlaveAddress);
    USI_TWI_send(GOFi2cOLED_Data_Mode);
    for (uint8_t k=0;k<16;k++){
      USI_TWI_send( 0 );
    }
    USI_TWI_endTransmission();
  }
}

/*
void ssd1306_displayX(int off) {

  ssd1306_startScreen();
    
  for (uint16_t i=0; i<=((128*32/8)/16); i++) 
  {
    // send a bunch of data in one xmission
    USI_TWI_beginTransmission(SlaveAddress);
    USI_TWI_send(GOFi2cOLED_Data_Mode);
    for (uint8_t k=0;k<16;k++){
      USI_TWI_send( i*16 + k + off);
    }
    USI_TWI_endTransmission();
  }
}*/


int ssd1306_getFontWidth(void) {
  return currentFont.width;
}

void ssd1306_printChar(char c) {
  uint16_t charSize = currentFont.charSize;
  uint16_t ch = (c - currentFont.first_char) * charSize;

  sendFlashData(&currentFont.data[ch], charSize);
}

// todo: get col and row internally
// col: 0 to 127
// row: 0 to 31
void ssd1306_printString(uint8_t col, uint8_t row, const char * pText) {
  unsigned char i;
  unsigned char len = strlen( pText );
  uint8_t width = currentFont.width;
  uint8_t pages_height = currentFont.height >> 3;
  uint8_t page = row >> 3; // divide by 8 to get page number

  for (i=0; i<len; i++, col+=currentFont.width)
  {
    ssd1306_clipArea( col, page, width, pages_height);
    ssd1306_printChar( pText[i] );
  }
}


void ssd1306_printNumber(uint8_t col, uint8_t row, int num) {
  char str[16];
  itoa(num, str, 10);
  ssd1306_printString(col, row, str);
}

// col (horizontal): value from 0 to (128-6*lenght) in which to print data
// row (vertical): value from 0 to 3
// num: float number divided in two integers
// decimal_digits: desired decimal places to print
// min_len: minimum lenght of string, filled with spaces if necesary (could be zero)
void ssd1306_printFloat(uint8_t col, uint8_t row, const float32_t num, 
                          uint8_t decimal_digits, uint8_t min_len) {
  char str[16] = "";
  char aux[8] = "", aux2[8] = "";

  if(num.sign == -1)
    str[0] = '-';

  itoa((int) num.integer, aux, 10);

  strncat(str, aux, sizeof aux - 1);
  strcat(str, ".");

  itoa((int) num.decimal, aux, 10);

  uint8_t decimal_len = strnlen(aux, sizeof aux - 1); // actual decimal digits
  uint8_t pad = 4 - decimal_len; // 4 is maximum expected decimal digits

  if((decimal_len < decimal_digits) && (pad > 0))
  {	
    memset(aux2, '0', pad);
    strcat(aux2, aux);
    strcpy(aux, aux2);
  }

  strncat(str, aux, decimal_digits);
  uint8_t len = strnlen(str, sizeof str - 1);

  if(min_len > sizeof str - 1)
    min_len = sizeof str - 1;
    
  if(len < min_len) {
    // Fill with spaces to a fixed lenght, so if the number changes it's lenght
    // over time, the spaces will clear old digits.
    uint8_t padding = min(min_len - len, sizeof str - len - 1);
    memset(&str[len], ' ', padding);
    len += padding;
    str[len] = '\0';
  }

  ssd1306_printString(col, row, str);  
}

void ssd1306_drawImage( const uint8_t * _bitmap, uint8_t col, uint8_t row ) {
  const bitmap_t *bitmap = (bitmap_t*) _bitmap;
  const uint8_t w = pgm_read_byte(&bitmap->width);
  const uint8_t h = pgm_read_byte(&bitmap->height) >> 3; // should ensure to round up
  const uint8_t *img = &bitmap->data;
  
  ssd1306_clipArea( col, row, w, h);
  
  sendFlashData(img, w*h);
}

char* itoa(int value, char* result, int base) {
    // check that the base if valid
    if (base < 2 || base > 36) { *result = '\0'; return result; }

    char* ptr = result, *ptr1 = result, tmp_char;
    int tmp_value;

    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
    } while ( value );

    // Apply negative sign
    if (tmp_value < 0) *ptr++ = '-';
    *ptr-- = '\0';
    while(ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr--= *ptr1;
        *ptr1++ = tmp_char;
    }
    return result;
}
