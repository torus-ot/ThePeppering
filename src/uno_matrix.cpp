#include "uno_matrix.h"

extern ArduinoLEDMatrix matrix; 



const uint32_t* icon = nullptr;
const uint32_t EXCLAMATION_LEFT[] = {0x30030030, 0x3003000, 0x300300};
const uint32_t EXCLAMATION_RIGHT[] = {0xc00c00, 0xc00c00c0, 0xc00c};
const uint32_t EXCLAMATION_BOTH[] = {0x30c30c30, 0xc30c30c0, 0x30c30c};

const uint32_t m_peppering[][3] = {
    {0x00000, 0x00002496, 0xdb000fff}, //   M_0_0_0_0
    {0x00100, 0x10012496, 0xdb000fff}, //   M_0_0_0_1
    {0x00800, 0x80082496, 0xdb000fff}, //   M_0_0_1_0
    {0x00900, 0x90092496, 0xdb000fff}, //   M_0_0_1_1

    {0x04004, 0x00402496, 0xdb000fff}, //   M_0_1_0_0
    {0x04104, 0x10412496, 0xdb000fff}, //   M_0_1_0_1
    {0x04804, 0x80482496, 0xdb000fff}, //   M_0_1_1_0
    {0x04904, 0x90492496, 0xdb000fff}, //   M_0_1_1_1

    {0x20020, 0x02002496, 0xdb000fff}, //   M_1_0_0_0
    {0x20120, 0x12012496, 0xdb000fff}, //   M_1_0_0_1
    {0x20820, 0x82082496, 0xdb000fff}, //   M_1_0_1_0
    {0x20920, 0x92092496, 0xdb000fff}, //   M_1_0_1_1

    {0x24024, 0x02402496, 0xdb000fff}, //   M_1_1_0_0
    {0x24124, 0x12412496, 0xdb000fff}, //   M_1_1_0_1
    {0x24824, 0x82482496, 0xdb000fff}, //   M_1_1_1_0
    {0x24924, 0x92492496, 0xdb000fff}  //   M_1_1_1_1
};

void ShowIconById(IconId iconId) {
 
  switch (iconId) {
    case ICON_SMILEY:
      icon = LEDMATRIX_EMOJI_HAPPY;
      break;
    case ICON_EXCLAM_LEFT:
      icon = EXCLAMATION_LEFT;
      break;
    case ICON_EXCLAM_RIGHT:
      icon = EXCLAMATION_RIGHT;
      break;
    case ICON_EXCLAM_BOTH:
      icon = EXCLAMATION_BOTH;
      break;
    default:
      matrix.clear();  // Safe fallback for invalid ID
      return;
  }
  // we are here only if the icon was assigned
  matrix.loadFrame(icon);
}

void printIcon12x8(const uint32_t icon[]) {
  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 12; col++) {
      int bitIndex = row * 12 + col;
      int wordIndex = bitIndex / 32;
      int bitInWord = 31 - (bitIndex % 32);
      bool bit = (icon[wordIndex] >> bitInWord) & 0x1;
      Serial.print(bit ? '#' : '.');
    }
    Serial.println();
  }
}

int getSensorPatternIndex(const int* dgValues, int count) {
  // 4 valus in -- int valuee 0-16 is returned
  int index = 0;
  for (int i = 0; i < count && i < 4; i++) {
      // Or this will work too:
      // index <<= 1;
      // index |= (dgValues[i] & 1);
      index <<= 1;
      index |= (dgValues[i] ? 1 : 0);
  }
  return index;  // Value from 0 to 15
}