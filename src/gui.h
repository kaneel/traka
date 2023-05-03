#ifndef __GUI_H__
#define __GUI_H__

#include "SDL.h"
#include "SDL_stdinc.h"
#include <stdio.h>
#include <stdbool.h>

#include <SDL.h>

#define NANOVG_GL3_IMPLEMENTATION
#include "../../nanovg/src/nanovg.h"

typedef char * AssetDefinition[2];

typedef enum Elements {
  Box,
  Button,
  Input,
} Elements;

typedef struct GUIElementState {
  bool pressed;
  bool hover;
  bool focus;
} GUIElementState;

typedef struct GUIElement {
  Elements type;
  float dimensions[4];
  char * text;
  short numericValue;
  GUIElementState state;
} GUIElement;

void convert_to_string(char * buffer, short value);
void calculateTextMetrics(char * text, float dimensions[4], float t_dimensions[4]);

void GUI_toggle_debug();
void GUI_DebugBox(float x, float y, float w, float h);
void GUI_Slider(GUIElement * element, Uint32, Uint32);
bool GUI_behaviour(GUIElement * element, bool force_false);
bool GUI_Button(GUIElement * element, bool is_pressed);
bool GUI_TextButton(GUIElement * element, bool is_pressed);
bool GUI_Input(GUIElement * element, bool is_focused);

void load_fonts(AssetDefinition *, short);

#endif
