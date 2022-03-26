/**
 * Copyright 2022 Jack Higgins : https://github.com/skhg
 * All components of this project are licensed under the MIT License.
 * See the LICENSE file for details.
 */

// Inspired by http://static.fontstruct.com/fontstructions/show/1888398/fs-tahoma-8px-9

#ifndef GLYPHS_H_
#define GLYPHS_H_

const byte GLYPH_ON_AIR_BLOCK_0[] = {
  B01110011,
  B10001011,
  B10001010,
  B10001010,
  B10001010,
  B10001010,
  B10001010,
  B01110010
};

const byte GLYPH_ON_AIR_BLOCK_1[] = {
  B00010000,
  B00010000,
  B10010000,
  B10010000,
  B01010000,
  B01010000,
  B00110000,
  B00110000
};

const byte GLYPH_ON_AIR_BLOCK_2[] = {
  B00110001,
  B00110000,
  B01001000,
  B01001000,
  B01001000,
  B11111100,
  B10000100,
  B10000101
};

const byte GLYPH_ON_AIR_BLOCK_3[] = {
  B11011110,
  B10010001,
  B10010001,
  B10010001,
  B10011110,
  B10010010,
  B10010001,
  B11010001
};

const byte GLYPH_CLOCK_SEPARATOR[] = {
  B00000000,
  B00000000,
  B00000001,
  B00000000,
  B00000000,
  B00000001,
  B00000000,
  B00000000
};

const byte GLYPH_EMPTY[] = {
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000
};

const byte GLYPH_NUMBER_0[] = {
  B00111000,
  B01000100,
  B01000100,
  B01000100,
  B01000100,
  B01000100,
  B01000100,
  B00111000
};

const byte GLYPH_NUMBER_1[] = {
  B00010000,
  B00110000,
  B00010000,
  B00010000,
  B00010000,
  B00010000,
  B00010000,
  B00111000
};

const byte GLYPH_NUMBER_2[] = {
  B00111000,
  B01000100,
  B00000100,
  B00001000,
  B00010000,
  B00100000,
  B01000000,
  B01111100
};

const byte GLYPH_NUMBER_3[] = {
  B00111000,
  B01000100,
  B00000100,
  B00011000,
  B00000100,
  B00000100,
  B01000100,
  B00111000
};

const byte GLYPH_NUMBER_4[] = {
  B00001000,
  B00011000,
  B00101000,
  B01001000,
  B01111100,
  B00001000,
  B00001000,
  B00001000
};

const byte GLYPH_NUMBER_5[] = {
  B01111100,
  B01000000,
  B01000000,
  B01111000,
  B00000100,
  B00000100,
  B01000100,
  B00111000
};

const byte GLYPH_NUMBER_6[] = {
  B00011000,
  B00100000,
  B01000000,
  B01111000,
  B01000100,
  B01000100,
  B01000100,
  B00111000
};

const byte GLYPH_NUMBER_7[] = {
  B01111100,
  B00000100,
  B00001000,
  B00001000,
  B00010000,
  B00010000,
  B00100000,
  B00100000
};

const byte GLYPH_NUMBER_8[] = {
  B00111000,
  B01000100,
  B01000100,
  B00111000,
  B01000100,
  B01000100,
  B01000100,
  B00111000
};

const byte GLYPH_NUMBER_9[] = {
  B00111000,
  B01000100,
  B01000100,
  B01000100,
  B00111100,
  B00000100,
  B00001000,
  B00110000
};

const byte *getGlyphForInteger(int number) {
  switch (number) {
    case 0: return GLYPH_NUMBER_0;
    case 1: return GLYPH_NUMBER_1;
    case 2: return GLYPH_NUMBER_2;
    case 3: return GLYPH_NUMBER_3;
    case 4: return GLYPH_NUMBER_4;
    case 5: return GLYPH_NUMBER_5;
    case 6: return GLYPH_NUMBER_6;
    case 7: return GLYPH_NUMBER_7;
    case 8: return GLYPH_NUMBER_8;
    case 9: return GLYPH_NUMBER_9;
    default: return GLYPH_EMPTY;
  }
}

#endif  // GLYPHS_H_