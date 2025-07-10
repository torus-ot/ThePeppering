#include "uno_matrix.h"

extern ArduinoLEDMatrix matrix; 



const uint32_t* icon = nullptr;
const uint32_t EXCLAMATION_LEFT[] = {0x30030030, 0x3003000, 0x300300};
const uint32_t EXCLAMATION_RIGHT[] = {0xc00c00, 0xc00c00c0, 0xc00c};
const uint32_t EXCLAMATION_BOTH[] = {0x30c30c30, 0xc30c30c0, 0x30c30c};

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
