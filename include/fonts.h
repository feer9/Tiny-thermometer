#ifndef _FONTS_H
#define _FONTS_H

#include <stdint.h>
#include <avr/pgmspace.h>

// Font structure as saved in flash memory
typedef struct {
  uint8_t type;
  uint8_t width;
  uint8_t height;
  uint8_t first_char;
  uint8_t data;
} font_t;

// Current font settings stored in SRAM
typedef struct {
  uint8_t width;
  uint8_t height;
  uint8_t first_char;
  uint16_t charSize;
  const uint8_t *data; // pointer to font_t[4], where actual characters start
} fontSettings;

// Fonts
extern const PROGMEM uint8_t ssd1306xled_font5x7_AB[];
extern const PROGMEM uint8_t comic_sans_font24x32_123[];
extern const PROGMEM uint8_t ssd1306xled_font8x16[];
extern const PROGMEM uint8_t CantarellExtraBold_12x16[];

#endif // _FONTS_H