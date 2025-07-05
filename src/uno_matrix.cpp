#include "uno_matrix.h"


void ShowIconById(IconId iconId) {
  const uint32_t* icon = nullptr;

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

  matrix.loadFrame(icon);
}
