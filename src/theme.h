#ifndef __THEME_H__
#define __THEME_H__

#include "../../nanovg/src/nanovg.h"

struct Theme {
  NVGcolor boxBackground;
  NVGcolor boxHover;
  NVGcolor boxStroke;
  NVGcolor buttonBackground;
  NVGcolor buttonBackgroundHover;
  NVGcolor textColour;
  NVGcolor textActiveColour;
  NVGcolor cursorBackground;
  NVGcolor cursorTextColour;
  NVGcolor playHeadColour;
  NVGcolor inputBackground;
  NVGcolor beatHighlight;
  NVGcolor ghostColour;
  NVGcolor graphAxis;
  NVGcolor graphLine;
  NVGcolor graphMarkers;
} theme;

void init_theme();

#endif
