#include "SDL.h"
#include "SDL_audio.h"
#include "SDL_stdinc.h"
#include <stdbool.h>
#include <OpenGL/gl3.h>
#include <stdint.h>

#ifndef NANOVG_GL3_IMPLEMENTATION
#define NANOVG_GL3_IMPLEMENTATION
#include "../../nanovg/src/nanovg.h"
#include "../../nanovg/src/nanovg_gl.h"
#endif

#include "./app.h"

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const char * WINDOW_TITLE = "TRAKA";

int set_audio_device(int device_id, SDL_AudioCallback audio_callback, CallbackData * callbackData) {
  if (app.audio_device_id) {
    SDL_CloseAudioDevice(app.audio_device_id);
  }

  SDL_AudioSpec audio_spec_want, audio_spec;
  SDL_memset(&audio_spec_want, 0, sizeof(audio_spec_want));

  audio_spec_want.freq     = 44100;
  audio_spec_want.format   = AUDIO_F32SYS;
  audio_spec_want.channels = 2;
  audio_spec_want.samples  = 1024;
  audio_spec_want.callback = audio_callback;
  audio_spec_want.userdata = callbackData;

  app.audio_device_id = SDL_OpenAudioDevice(
    device_id == -1 ? NULL : SDL_GetAudioDeviceName(device_id, false),
    0,
    &audio_spec_want, 
    &audio_spec,
    0
  );

  if (audio_spec.format != audio_spec_want.format) {
    printf("WARNING: not my audio format here: %d", audio_spec.format);
  }

  if(!app.audio_device_id) {
    fprintf(stderr,
      "Error creating SDL audio device. SDL_Error: %s\n",
      SDL_GetError()
    );
    destroy_app(app);
    exit(1);
  } 

  return app.audio_device_id;
}

void init_app(int argc, char *argv[]) {
  for (short i = 0; i < argc; i++) {
    if (strcmp(argv[i] , "-debug") == 0) {
      app.debug = true;
    }
  }

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
    printf("Couldn't initialize SDL: %s\n", SDL_GetError());
    exit(1);
  }

  int context_flags = SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG;

  if (app.debug) {
    context_flags |= SDL_GL_CONTEXT_DEBUG_FLAG;
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, context_flags);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  app.window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);

  if (!app.window) {
    printf("Failed to open %d x %d window: %s\n", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_GetError());
    destroy_app(app);
    exit(1);
  }

  app.context = SDL_GL_CreateContext(app.window);

  if (!app.context) {
    printf("Failed to create gl context: %s\n", SDL_GetError());
    destroy_app(app);
    exit(1);
  } else {
    SDL_GL_MakeCurrent(app.window, app.context);
  }

  // INITIALIZE NANOVG:
  int flags = NVG_STENCIL_STROKES | NVG_ANTIALIAS;

  if (app.debug) {
    flags |= NVG_DEBUG;
  }

  app.nvg = nvgCreateGL3(flags);
  if (!app.nvg) {
    printf("Failed to create NVG Context");
    destroy_app(app);
    exit(1);
  }
}

void destroy_app() {
  SDL_CloseAudioDevice(app.audio_device_id);
  if (app.nvg) nvgDeleteGL3(app.nvg);
  if (app.context) SDL_GL_DeleteContext(app.context);
  if (app.window) SDL_DestroyWindow(app.window);
  SDL_Quit();
}

