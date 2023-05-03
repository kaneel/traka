#include <unistd.h>
#include "app.h"
#include "audio.h"
#include "gui.h"
#include "theme.h"
#include "sample.h"

#define AVAIL_WIDTH 680.0
#define LEFT_COL_WIDTH 100.0
#define MIDDLE 600.0/2.0
#define SAMPLE_HEIGHT 200.0

GUIElement LoopOffButton = {
  .type = Button,
  .dimensions = { 100, 540, 50, 22 },
  .text = "Off",
  .state = { 0, 0, 0 }
};
GUIElement LoopForwardButton = {
  .type = Button,
  .dimensions = { 150, 540, 80,  22 },
  .text = "Forward",
  .state = { 0, 0, 0 }
};
GUIElement LoopPingPongButton = {
  .type = Button,
  .dimensions = { 230, 540, 130,  22 },
  .text = "Ping Pong",
  .state = { 0, 0, 0 }
};
GUIElement LoopReverseButton = {
  .type = Button,
  .dimensions = { 360, 540, 100,  22 },
  .text = "Reverse",
  .state = { 0, 0, 0 }
};
GUIElement SliderStart = {
  .type = Button,
  .text = "S",
  .state = { 0, 0, 0 }
};
GUIElement SliderEnd = {
  .type = Button,
  .text = "E",
  .state = { 0, 0, 0 }
};

Sample load_sample(char path[256], char name[256], char filename[256]) {
  SDL_AudioSpec wav_spec;
  Uint32 wav_len;
  Uint8 *wav_buffer;

  SDL_AudioSpec* wave = SDL_LoadWAV(path, &wav_spec, &wav_buffer, &wav_len);

  if (!wave) {
    fprintf(stderr, "Could not open %s: %s\n", path, SDL_GetError());
    exit(1);
  } 

  short bit_depth = SDL_AUDIO_BITSIZE(wav_spec.format);

  Sample sample = {
    .wav_spec = wav_spec,
    .bit_depth = bit_depth,
    .name = name,
    .filename = filename,
  };

  SDL_AudioCVT cvt;

  if (bit_depth == 8) {
    SDL_BuildAudioCVT(&cvt, AUDIO_U8, wav_spec.channels, wav_spec.freq, AUDIO_F32, wav_spec.channels, wav_spec.freq);
  } else if (bit_depth == 16) {
    SDL_BuildAudioCVT(&cvt, AUDIO_S16, wav_spec.channels, wav_spec.freq, AUDIO_F32, wav_spec.channels, wav_spec.freq);
  } else if (bit_depth == 32) {
    SDL_BuildAudioCVT(&cvt, AUDIO_S32, wav_spec.channels, wav_spec.freq, AUDIO_F32, wav_spec.channels, wav_spec.freq);
  }

  cvt.len = wav_len * wav_spec.channels;
  cvt.buf = (Uint8 *) SDL_malloc(cvt.len * cvt.len_mult);
  for (Uint32 k = 0; k < wav_len; k++) {
    cvt.buf[k] = wav_buffer[k];
  }
  SDL_ConvertAudio(&cvt);
  sample.wav_len = cvt.len_cvt / 4;
  sample.wav_buffer = (float *)cvt.buf;

  return sample;
}

void load_samples(SampleList * smps, SampleDefinition * smps_list, short len) {
  char cwd[256];
  getcwd(cwd, 256);

  for (short i = 0; i < len; i++) {
    char sample_path[256] = "";

    strcat(sample_path, cwd);
    strcat(sample_path, "/build/samples/");
    strcat(sample_path, smps_list[i].filename);

    Sample sample = load_sample(sample_path, smps_list[i].name, smps_list[i].filename);

    if (smps_list[i].loop.mode == 0) {
      sample.loop = (Loop){
        .mode = Loop_OFF,
        .start = 0,
        .end = sample.wav_len
      };
    } else {
      sample.loop = smps_list[i].loop;
    }
    smps->list[smps->count++] = sample;
  }
}

void samples_keyboard(SDL_Keycode key) {
  switch (key) {
    case SDLK_DOWN:
      app_state.current_sample += 1;
      break;
    case SDLK_UP:
      if (app_state.current_sample > 0) {
        app_state.current_sample -= 1;
      }
      break;
    
  }
}

void draw_sample_list(SampleList * samples) {
  nvgBeginPath(app.nvg);
  nvgFontSize(app.nvg, 14.0f);
  nvgFillColor(app.nvg, theme.textColour);

  for (short i = 0; i < samples->count; i++) {
    if (i == app_state.current_sample) {
      nvgText(app.nvg, 10, 100 + (i * 14), ">", NULL);
    }

    nvgText(app.nvg, 20, 100 + (i * 14), samples->list[i].name, NULL);
  }
}

void draw_sample(Sample * sample, PlaybackList * playbacks) {
  nvgBeginPath(app.nvg);
  nvgStrokeWidth(app.nvg, 1.0);

  /*
   * GRAPH X AXIS 
   */
  nvgStrokeColor(app.nvg, theme.graphAxis);
  nvgMoveTo(app.nvg, LEFT_COL_WIDTH, MIDDLE);
  nvgLineTo(app.nvg, LEFT_COL_WIDTH + AVAIL_WIDTH, MIDDLE);
  nvgStroke(app.nvg);

  nvgBeginPath(app.nvg);
  nvgStrokeColor(app.nvg, theme.graphLine);
  nvgStrokeWidth(app.nvg, 1.0);

  nvgMoveTo(app.nvg, LEFT_COL_WIDTH, MIDDLE);

  float ratio = sample->wav_len / AVAIL_WIDTH;
  float ratioS = AVAIL_WIDTH / sample->wav_len;
  /*
   * draw the sample
   */
  for (Uint32 i = 0; i < AVAIL_WIDTH; i++) {
    Uint32 index = floor(i*ratio);
    nvgLineTo(app.nvg, LEFT_COL_WIDTH + i, MIDDLE + sample->wav_buffer[index] * SAMPLE_HEIGHT);
  }

  nvgStroke(app.nvg);

  /*
   * draw the loop markers
   */
  if (sample->loop.mode != Loop_OFF) {
    Uint32 start = floor((float)sample->loop.start * ratioS);
    Uint32 end = floor((float)sample->loop.end * ratioS);

    nvgBeginPath(app.nvg);
    nvgStrokeColor(app.nvg, theme.graphMarkers);
    nvgFillColor(app.nvg, theme.graphMarkers);

    nvgMoveTo(app.nvg, start + LEFT_COL_WIDTH, MIDDLE - SAMPLE_HEIGHT);
    nvgLineTo(app.nvg, start + LEFT_COL_WIDTH, MIDDLE + SAMPLE_HEIGHT);
    nvgStroke(app.nvg);

    SliderStart.dimensions[0] = start + LEFT_COL_WIDTH;
    SliderStart.dimensions[1] = MIDDLE + SAMPLE_HEIGHT - 20;
    SliderStart.dimensions[2] = 20;
    SliderStart.dimensions[3] = 20;
    GUI_Button(&SliderStart, false);
    GUI_Slider(&SliderStart, 100, AVAIL_WIDTH + 100);

    nvgBeginPath(app.nvg);
    nvgStrokeColor(app.nvg, theme.graphMarkers);
    nvgMoveTo(app.nvg, LEFT_COL_WIDTH + end, MIDDLE - SAMPLE_HEIGHT);
    nvgLineTo(app.nvg, LEFT_COL_WIDTH + end, MIDDLE + SAMPLE_HEIGHT);
    nvgStroke(app.nvg);

    SliderEnd.dimensions[0] = LEFT_COL_WIDTH + end - 20;
    SliderEnd.dimensions[1] = MIDDLE + SAMPLE_HEIGHT - 20;
    SliderEnd.dimensions[2] = 20;
    SliderEnd.dimensions[3] = 20;
    GUI_Button(&SliderEnd, false);
    GUI_Slider(&SliderEnd, 100, AVAIL_WIDTH + 100);

    float start_pos = SliderStart.dimensions[0] - LEFT_COL_WIDTH;
    float end_pos = SliderEnd.dimensions[0] - LEFT_COL_WIDTH + 20;
    Uint32 final_s = floor(start_pos / ratioS);
    Uint32 final_e = floor(end_pos / ratioS);

    if (start_pos != start) {
      sample->loop.start = MAX(0, final_s);
    }

    if (end_pos != end) {
      sample->loop.end = MIN(sample->wav_len, final_e);
    }
  }

  /*
   * draw playback heads
   */
  for (short i = 0; i < playbacks->count; i++) {
    Playback playback = playbacks->list[i];

    if (!playback.set || playback.sample != sample) {
      break;
    }

    Uint32 cursor = floor((float)playback.play_cursor * ratioS);
    nvgBeginPath(app.nvg);
    nvgStrokeColor(app.nvg, theme.graphMarkers);
    nvgMoveTo(app.nvg, LEFT_COL_WIDTH + cursor, MIDDLE - SAMPLE_HEIGHT);
    nvgLineTo(app.nvg, LEFT_COL_WIDTH + cursor, MIDDLE + SAMPLE_HEIGHT);
    nvgStroke(app.nvg);
  }
}

void draw_sample_ui(Sample * sample) {
  if (GUI_Button(&LoopOffButton, sample->loop.mode == Loop_OFF)) {
    sample->loop.mode = Loop_OFF;
  }

  if (GUI_Button(&LoopForwardButton, sample->loop.mode == Loop_FORWARD)) {
    sample->loop.mode = Loop_FORWARD;
  }

  if (GUI_Button(&LoopPingPongButton, sample->loop.mode == Loop_PINGPONG)) {
    sample->loop.mode = Loop_PINGPONG;
  }

  if (GUI_Button(&LoopReverseButton, sample->loop.mode == Loop_REVERSE)) {
    sample->loop.mode = Loop_REVERSE;
  }
}

void draw_sample_view(SampleList * samples, PlaybackList * playbacks) {
  draw_sample_list(samples);
  draw_sample(&samples->list[app_state.current_sample], playbacks);
  draw_sample_ui(&samples->list[app_state.current_sample]);
}
