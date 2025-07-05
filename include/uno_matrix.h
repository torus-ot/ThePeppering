#ifndef UNO_MATRIX
#define UNO_MATRIX
#include <Arduino.h>
#include <Arduino_LED_Matrix.h>

ArduinoLEDMatrix matrix;

enum IconId {
  ICON_SMILEY = 0,
  ICON_EXCLAM_LEFT = 1,
  ICON_EXCLAM_RIGHT = 2,
  ICON_EXCLAM_BOTH = 3
};

inline const uint32_t EXCLAMATION_LEFT[] = {0x30030030, 0x3003000, 0x300300};
inline const uint32_t EXCLAMATION_RIGHT[] = {0xc00c00, 0xc00c00c0, 0xc00c};
inline const uint32_t EXCLAMATION_BOTH[] = {0x30c30c30, 0xc30c30c0, 0x30c30c};

void ShowIconById(IconId iconId);

 #endif