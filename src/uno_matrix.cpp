#include "uno_matrix.h"

extern ArduinoLEDMatrix matrix; 



const uint32_t* icon = nullptr;
const uint32_t EXCLAMATION_LEFT[] = {0x30030030, 0x3003000, 0x300300};
const uint32_t EXCLAMATION_RIGHT[] = {0xc00c00, 0xc00c00c0, 0xc00c};
const uint32_t EXCLAMATION_BOTH[] = {0x30c30c30, 0xc30c30c0, 0x30c30c};

// Legend for per sensor 2x4 blocks (Columns from left to right: S1, S2, S3, S4)
constexpr uint32_t M_0_0_0_0[] = { 0x00000000, 0x00F000F0, 0x00F000F0 };
constexpr uint32_t M_0_0_0_1[] = { 0x00000000, 0x00F000F0, 0x10F010F0 };
constexpr uint32_t M_0_0_1_0[] = { 0x00000000, 0x00F010F0, 0x00F000F0 };
constexpr uint32_t M_0_0_1_1[] = { 0x00000000, 0x00F010F0, 0x10F010F0 };

constexpr uint32_t M_0_1_0_0[] = { 0x00000000, 0x10F000F0, 0x00F000F0 };
constexpr uint32_t M_0_1_0_1[] = { 0x00000000, 0x10F000F0, 0x10F010F0 };
constexpr uint32_t M_0_1_1_0[] = { 0x00000000, 0x10F010F0, 0x00F000F0 };
constexpr uint32_t M_0_1_1_1[] = { 0x00000000, 0x10F010F0, 0x10F010F0 };

constexpr uint32_t M_1_0_0_0[] = { 0x00000000, 0x00F000F0, 0x00F100F1 };
constexpr uint32_t M_1_0_0_1[] = { 0x00000000, 0x00F000F0, 0x10F110F1 };
constexpr uint32_t M_1_0_1_0[] = { 0x00000000, 0x00F010F0, 0x00F100F1 };
constexpr uint32_t M_1_0_1_1[] = { 0x00000000, 0x00F010F0, 0x10F110F1 };

constexpr uint32_t M_1_1_0_0[] = { 0x00000000, 0x10F000F0, 0x00F100F1 };
constexpr uint32_t M_1_1_0_1[] = { 0x00000000, 0x10F000F0, 0x10F110F1 };
constexpr uint32_t M_1_1_1_0[] = { 0x00000000, 0x10F010F0, 0x00F100F1 };
constexpr uint32_t M_1_1_1_1[] = { 0x00000000, 0x10F010F0, 0x10F110F1 };


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
