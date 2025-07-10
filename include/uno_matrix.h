#ifndef UNO_MATRIX
#define UNO_MATRIX
#include <Arduino.h>
#include <Arduino_LED_Matrix.h>

enum IconId {
  ICON_SMILEY = 0,
  ICON_EXCLAM_LEFT = 1,
  ICON_EXCLAM_RIGHT = 2,
  ICON_EXCLAM_BOTH = 3
};

void ShowIconById(IconId iconId);
void printIcon12x8(const uint32_t icon[]);
 #endif