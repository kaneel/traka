#include <SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include "./theme.h"
#include "./app.h"
#include "./gui.h"

static GUIElement * pressed_el = NULL;

void convert_to_string(char * buffer, short value) {
  snprintf(buffer, 4, "%d", value);
}

bool mouse_was_down;
bool GUI_behaviour(GUIElement * element, bool force_false) {
  float * dimensions = element->dimensions;
  int mouse_x, mouse_y;
  Uint32 mouse_buttons = SDL_GetMouseState(&mouse_x, &mouse_y);

  bool mouse_down = (mouse_buttons & SDL_BUTTON_LMASK) != 0;
  bool pressed = mouse_down && !mouse_was_down;
  bool ret = false;

  if (!mouse_down) {
    pressed_el = NULL;
  }

  if (mouse_x > dimensions[0] && mouse_x < (dimensions[0] + dimensions[2]) &&
      mouse_y > dimensions[1] && mouse_y < (dimensions[1] + dimensions[3])) {
    ret = pressed;
    element->state.hover = true;
    mouse_was_down = mouse_down;
    pressed_el = mouse_down ? element : NULL;
  } else {
    element->state.hover = false;
  }

  element->state.pressed = pressed_el == element;

  return force_false ? false : ret;
}

bool GUI_Slider(GUIElement * element) {
  bool clicked = GUI_behaviour(element, false);

  int x;
  if (element->state.pressed) {
    SDL_GetMouseState(&x, NULL);
    element->dimensions[0] = x;
  }

  nvgRect(app.nvg, element->dimensions[0], element->dimensions[1], element->dimensions[2], element->dimensions[3]);

  nvgFillColor(app.nvg, theme.graphMarkers);
  nvgStrokeWidth(app.nvg, 2.f);
  nvgFill(app.nvg);

  nvgFontFace(app.nvg, "regular");
  nvgFontSize(app.nvg, 22.0f);
  nvgTextAlign(app.nvg, NVG_ALIGN_LEFT|NVG_ALIGN_MIDDLE);

  nvgFillColor(app.nvg, theme.textColour);
  nvgText(app.nvg, element->dimensions[0], element->dimensions[1] + 10, element->text, NULL);

  return clicked;
}


bool GUI_Button(GUIElement element, bool is_pressed) {
  float * dimensions = element.dimensions;
  bool clicked = GUI_behaviour(&element, is_pressed);

  NVGcolor nvg_stroke_color = theme.boxStroke;
  NVGcolor nvg_fill_color = is_pressed || element.state.hover ? theme.buttonBackgroundHover : theme.buttonBackground;
  
  nvgBeginPath(app.nvg); 

  nvgFontFace(app.nvg, "regular");
  nvgFontSize(app.nvg, 22.0f);
  nvgTextAlign(app.nvg, NVG_ALIGN_LEFT|NVG_ALIGN_MIDDLE);

  int y = dimensions[1] + (is_pressed ? 2 : 0);

  nvgRect(app.nvg, dimensions[0], y, dimensions[2], dimensions[3]);

  nvgFillColor(app.nvg, nvg_fill_color);
  nvgStrokeWidth(app.nvg, 2.f);
  nvgStrokeColor(app.nvg, nvg_stroke_color);
  nvgStroke(app.nvg);
  nvgFill(app.nvg);

  nvgFillColor(app.nvg, theme.textColour);
  nvgText(app.nvg, dimensions[0], y + 10, element.text, NULL);

  return clicked;
}

bool GUI_ComboBox(GUIElement element, bool is_pressed) {
  float * dimensions = element.dimensions;

  bool clicked = GUI_behaviour(&element, is_pressed);
  NVGcolor nvg_stroke_color = theme.boxStroke;
  NVGcolor nvg_fill_color = is_pressed || element.state.hover ? theme.buttonBackgroundHover : theme.buttonBackground;
  
  nvgBeginPath(app.nvg); 

  nvgFontFace(app.nvg, "regular");
  nvgFontSize(app.nvg, 22.0f);
  nvgTextAlign(app.nvg, NVG_ALIGN_LEFT|NVG_ALIGN_MIDDLE);

  nvgRect(app.nvg, dimensions[0], dimensions[1], dimensions[2], dimensions[3]);

  nvgFillColor(app.nvg, nvg_fill_color);
  nvgStrokeWidth(app.nvg, 2.f);
  nvgStrokeColor(app.nvg, nvg_stroke_color);
  nvgStroke(app.nvg);
  nvgFill(app.nvg);

  nvgFillColor(app.nvg, theme.textColour);
  nvgText(app.nvg, dimensions[0], dimensions[1] + 10, element.text, NULL);

  return clicked;
}

bool GUI_TextButton(GUIElement element, bool is_pressed) {
  nvgBeginPath(app.nvg); 
  float * dimensions = element.dimensions;
  float bounds[4] = { 0 };

  nvgFontFace(app.nvg, "regular");
  nvgFontSize(app.nvg, 22.0f);

  nvgTextBounds(app.nvg, element.dimensions[0], dimensions[1], element.text, NULL, bounds);

  dimensions[2] = bounds[2] - bounds[0];
  dimensions[3] = bounds[3] - bounds[1];

  bool clicked = GUI_behaviour(&element, is_pressed);

  nvgTextAlign(app.nvg, NVG_ALIGN_LEFT|NVG_ALIGN_MIDDLE);
  if (is_pressed)
    nvgFillColor(app.nvg, theme.textActiveColour);
  else if (element.state.hover)
    nvgFillColor(app.nvg, theme.textActiveColour);
  else
    nvgFillColor(app.nvg, theme.textColour);
  nvgText(app.nvg, element.dimensions[0], element.dimensions[1], element.text, NULL);

  return clicked;
}

bool GUI_Input(GUIElement element, bool is_focused) {
  float * dimensions = element.dimensions;
  bool clicked = GUI_behaviour(&element, is_focused);

  NVGcolor nvg_stroke_color = theme.boxStroke;
  NVGcolor nvg_fill_color = theme.inputBackground;
  
  nvgBeginPath(app.nvg); 

  nvgFontFace(app.nvg, "regular");
  nvgFontSize(app.nvg, 22.0f);
  nvgTextAlign(app.nvg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);

  int y = dimensions[1];

  nvgRect(app.nvg, dimensions[0], y, dimensions[2], dimensions[3]);

  nvgFillColor(app.nvg, nvg_fill_color);
  nvgStrokeWidth(app.nvg, 2.f);
  nvgStrokeColor(app.nvg, nvg_stroke_color);
  nvgStroke(app.nvg);
  nvgFill(app.nvg);

  char text[256];
  convert_to_string(text, element.numericValue);

  nvgFillColor(app.nvg, theme.textColour);
  nvgText(app.nvg, dimensions[0], y + 10, text, NULL);

  return clicked;
}

void load_fonts(AssetDefinition * fonts, short len) {
  char cwd[256];
  getcwd(cwd, 256);

  for (short i = 0; i < len; i++) {
    char font_path[256] = "";
    strcat(font_path, cwd);
    strcat(font_path, "/build/fonts/");
    strcat(font_path, fonts[i][1]);

    if (nvgCreateFont(app.nvg, fonts[i][0], font_path) < 0) {
      printf("Error loading font: %s\n", font_path);
      destroy_app(&app);
      exit(1);
    }
  }
}

