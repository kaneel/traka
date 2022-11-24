#include <SDL.h>
#include <stdbool.h>

#include <OpenGL/gl.h>
#include <OpenGL/gl3.h>

#include "./app.h"
#include "./gui.h"
#include "./theme.h"
#include "./tracker.h"
#include "./sample.h"

#define MINUTES_IN_MS 60.0 * 1000.0

int main(int argc, char *argv[]) {
  PatternColumn col = { .row_len = 16, .cursor = { 0, 0 }, .index = 0 };
  PatternColumn col2 = { .row_len = 8, .cursor = { -1, -1 }, .index = 1 };
  PatternColumn col3 = { .row_len = 24, .cursor = { -1, -1 }, .index = 2 };
  PatternColumn col4 = { .row_len = 32, .cursor = { -1, -1 }, .index = 3 };
  Pattern pat = { .columns = { [0] = &col, [1] = &col2, [2] = &col3, [3] = &col4 }, .column_count = 4 };
  Track track = { .patterns = { [0] = &pat }, .pattern_count = 1 };
  Cursor currentCursor = { .currentColumn = &col, .parentPattern = &pat };

  Channel channel = { .volume = 80, .panning = 40 };
  Channel channel2 = { .volume = 80, .panning = 40 };
  Channel channel3 = { .volume = 80, .panning = 40 };
  Channel channel4 = { .volume = 80, .panning = 40 };
  
  ChannelList channels = { .list = { channel, channel2, channel3, channel4 }, .count = 4 };
  PlaybackList playbacks = { .count = 0 };
  CallbackData callbackData = { .playbacks = &playbacks, .channels = &channels };

  init_theme();
  init_app(argc, argv);
  int audio_device_id = set_audio_device(-1, audio_callback, &callbackData);

  app_state = (AppState){ 
    .last_note_used = 1,
    .last_octave_used = 4,
    .last_instrument_used = 0,
    .last_volume_used = 0,
    .last_effect_number = 0,
    .last_effect_value = 0,
    .lpb = 4, 
    .bpm = 120,
    .is_playing = false,
    .startTime = 0,
    .song_position = -1,
    .audio_device_id = audio_device_id,
    .view = View_TRACKER,
    .visible_pattern_column = 0,
    .visible_pattern_row = 0,
    .current_sample = 0,
    .page_move = 10
  };

  SampleList smps = { .count = 0 };
  SampleDefinition smps_list[5] = { 
    { .name = "kick" , .filename = "kick.wav" },
    { 
      .name = "snare" , 
      .filename = "snare.wav", 
      .loop = {
        .mode = Loop_PINGPONG,
        .start = 500,
        .end = smps.list[1].wav_len - 1
      } 
    },
    { 
      .name = "hat" , 
      .filename = "hat.wav" ,
      .loop = {
        .mode = Loop_FORWARD,
        .start = 0,
        .end = smps.list[1].wav_len - 1
      }
    },
    { .name = "clap" , .filename = "clap.wav" },
    { 
      .name = "clapr" , 
      .filename = "clap_reduced.wav",
      .loop = {
        .mode = Loop_REVERSE,
        .start = 500,
        .end = smps.list[1].wav_len - 1
      }
    }
  };
  load_samples(&smps, smps_list, 5);

  AssetDefinition fonts[3] = { 
    { "regular" , "Inconsolata-Regular.ttf" },
    { "thin" , "Inconsolata-Thin.ttf" },
    { "black" , "Inconsolata-Black.ttf" }
  };
  load_fonts(fonts, 3);

  SDL_Event e;
  bool isRunning = 1;
  Uint32 currentTime;

  int was_playing = false;
  Uint32 prev_row = 0;

  while (isRunning) {
    currentTime = SDL_GetTicks();

    while (SDL_PollEvent(&e)) {
      switch(e.type) {
        case SDL_QUIT: {
          isRunning = 0;
          break;
        }

        case SDL_KEYDOWN: {
          if (e.key.keysym.sym == SDLK_ESCAPE) {
            isRunning = false;
            break;
          }

          if (e.key.keysym.sym == SDLK_F2) {
            app_state.view = View_TRACKER;
            break;
          }

          if (e.key.keysym.sym == SDLK_F3) {
            app_state.view = View_SAMPLES;
            break;
          }

          if (e.key.keysym.sym == SDLK_F4) {
            app_state.view = View_INSTRUMENTS;
            break;
          }

          if (e.key.keysym.sym == SDLK_F12) {
            app_state.view = View_SETTINGS;
            break;
          }

          if (e.key.keysym.sym == SDLK_F6) {
            app_state.is_playing = true;
            break;
          }

          if (e.key.keysym.sym == SDLK_F8) {
            app_state.is_playing = false;
            break;
          }
          
          switch(app_state.view) {
            case View_TRACKER: 
              tracker_keyboard(e.key.keysym.sym, &currentCursor);
              break;
            case View_SAMPLES: 
              samples_keyboard(e.key.keysym.sym);
              break;
            default: 
              break;
          }

          break;
        }
        case SDL_KEYUP: {
          break;
        }
      }
    }

    glClearColor(1.f, 1.f, 1.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    nvgBeginFrame(app.nvg, WINDOW_WIDTH, WINDOW_HEIGHT, 1.f);

    Pattern * pat = track.patterns[0];

    switch(app_state.view) {
      case View_TRACKER: 
        draw_pattern_view(&callbackData, pat, currentCursor);
        break;
      case View_SAMPLES: 
        draw_sample_view(&smps);
        break;
      default: 
        break;
    }

    if (app_state.is_playing && was_playing) {
      Uint32 elapsed = (currentTime - prev_row) - app_state.startTime;
      double row_ms = (MINUTES_IN_MS / app_state.bpm) / app_state.lpb;

      if (elapsed >= row_ms){ 
        prev_row += row_ms;
        app_state.song_position+=1;
        check_row(app_state.song_position, pat, &playbacks, &channels, &smps);
      }
    }

    if (app_state.is_playing && !was_playing) {
      SDL_PauseAudioDevice(app.audio_device_id, 0);
      was_playing = true;
      prev_row = 0;
      app_state.song_position=-1;
      app_state.startTime = currentTime;
    } else if (!app_state.is_playing && was_playing) {
      SDL_PauseAudioDevice(app.audio_device_id, 1);
      was_playing = false;
      playbacks = (PlaybackList){ .count = 0 };
    }

    nvgEndFrame(app.nvg);
    SDL_GL_SwapWindow(app.window);
  }

  destroy_app(&app);
  return 0;
}

