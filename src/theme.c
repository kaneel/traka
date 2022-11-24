#include "../../nanovg/src/nanovg.h"

#include "./theme.h"

void init_theme() {
  theme.boxBackground = nvgRGBAf(0.5f, 0.5f, 0.5f, 1.0f);
  theme.boxHover = nvgRGBAf(0.6f, 0.6f, 0.6f, 1.0f);
  theme.boxStroke = nvgRGBAf(0.f, 0.f, 0.f, 1.0f);
  theme.buttonBackground = nvgRGBAf(1.f, 0.5f, 0.5f, 1.0f);
  theme.buttonBackgroundHover = nvgRGBAf(1.f, 1.0f, 0.6f, 1.0f);
  theme.textColour = nvgRGBAf(0.f, 0.0f, 0.0f, 1.0f);
  theme.textActiveColour = nvgRGBAf(0.5f, 0.5f, 0.5f, 1.0f);
  theme.cursorBackground = nvgRGBAf(0.3f, 0.3f, 0.3f, 1.0f);
  theme.cursorTextColour = nvgRGBAf(1.0f, 1.0f, 1.0f, 1.0f);
  theme.playHeadColour = nvgRGBAf(0.0f, 0.0f, 0.0f, .4f);
  theme.inputBackground = nvgRGBAf(0.9f, 0.9f, 0.9f, 1.0f);
  theme.beatHighlight = nvgRGBAf(0.7f, 0.7f, 0.7f, 1.0f);
  theme.ghostColour = nvgRGBAf(0.6f, 0.6f, 0.6f, 1.0f);
  theme.graphAxis = nvgRGBAf(0.2f, 0.2f, 0.2f, 1.0f);
  theme.graphLine = nvgRGBAf(0.4f, 0.7f, 0.4f, 1.0f);
  theme.graphMarkers = nvgRGBAf(0.2f, 0.2f, 0.2f, 1.0f);
}
