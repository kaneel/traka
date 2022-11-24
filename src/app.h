#ifndef __APP_H__
#define __APP_H__

#include "SDL.h"
#include "SDL_audio.h"
#include "SDL_stdinc.h"
#include <stdbool.h>
#include <OpenGL/gl3.h>

#ifndef NANOVG_GL3_IMPLEMENTATION
#define NANOVG_GL3_IMPLEMENTATION
#include "../../nanovg/src/nanovg.h"
#endif

#include "audio.h"

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

const int WINDOW_WIDTH;
const int WINDOW_HEIGHT;
const char * WINDOW_TITLE;

typedef enum Views {
  View_TRACKER,
  View_SAMPLES,
  View_INSTRUMENTS,
  View_SETTINGS,
} Views;

typedef struct AppState {
  short last_note_used;
  short last_octave_used;
  short last_instrument_used;
  short last_volume_used;
  short last_effect_number;
  short last_effect_value;
  short bpm;
  short lpb;
  bool is_playing;
  int song_position;
  short visible_pattern_column;
  short page_move;
  short visible_pattern_row;
  short current_sample;
  SDL_AudioDeviceID audio_device_id;
  Uint32 startTime;
  Views view;
} AppState;

typedef struct App {
  SDL_Window * window;
  NVGcontext * nvg;
  SDL_GLContext * context;
  SDL_AudioDeviceID audio_device_id;
  SDL_AudioCallback audio_callback;
  bool debug;
} App;

App app;
AppState app_state;

int set_audio_device(int, SDL_AudioCallback, CallbackData *);
void init_app(int argc, char *argv[]);
void destroy_app();

#endif
