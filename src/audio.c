#include "audio.h"

float FREQUENCIES[12][9] = {
  { 16.35, 32.70, 65.41, 130.81, 261.63, 523.25, 1046.50, 2093.00, 4186.01 },
  { 17.32, 34.65, 69.30, 138.59, 277.18, 554.37, 1108.73, 2217.46, 4434.92 },
  { 18.35, 36.71, 73.42, 146.83, 293.66, 587.33, 1174.66, 2349.32, 4698.64 },
  { 19.45, 38.89, 77.78, 155.56, 311.13, 622.25, 1244.51, 2489.02, 4978.03 },
  { 20.60, 41.20, 82.41, 164.81, 329.63, 659.26, 1318.51, 2637.02 },
  { 21.83, 43.65, 87.31, 174.61, 349.23, 698.46, 1396.91, 2793.83 },
  { 23.12, 46.25, 92.50, 185.00, 369.99, 739.99, 1479.98, 2959.96 },
  { 24.50, 49.00, 98.00, 196.00, 392.00, 783.99, 1567.98, 3135.96 },
  { 25.96, 51.91, 103.83, 207.65, 415.30, 830.61, 1661.22, 3322.44 },
  { 27.50, 55.00, 110.00, 220.00, 440.00, 880.00, 1760.00, 3520.00 },
  { 29.14, 58.27, 116.54, 233.08, 466.16, 932.33, 1864.66, 3729.31 },
  { 30.87, 61.74, 123.47, 246.94, 493.88, 987.77, 1975.53, 3951.07 }
};

void audio_callback(void* userdata, unsigned char * s, int len) {
  CallbackData * callbackData = (CallbackData *)userdata;
  PlaybackList * playbacks = callbackData->playbacks;
  ChannelList * channels = callbackData->channels;

  float * stream = (float *)s;
  
  memset(stream, 0, len);

  for (short i = 0; i < playbacks->count; i++) {
    Playback p = playbacks->list[i];

    if(!p.set) {
      continue;
    }

    float * out = &stream[0];
    float * buffer_sample = p.sample->wav_buffer;
    Uint32 wav_len = p.sample->wav_len;
    int new_len = len / (sizeof(float)*2);

    Channel c = channels->list[p.channel];
    Sample * s = p.sample;

    float frequency = FREQUENCIES[p.note - 1][p.octave] / FREQUENCIES[0][4];
    float left_panning = (80.0 - c.panning);
    float right_panning = (80.0 - left_panning);
    float left_vol = (c.volume/80.0) * (left_panning/40.0);
    float right_vol = (c.volume/80.0) * (right_panning/40.0);
    float volume = p.volume / 64.0;

    Uint32 actual_cursor;
    Uint32 cursor;
    
    for (int a = 0; a < new_len; a++) {
      cursor = floor(a *  frequency);

      if (p.direction) {
        actual_cursor = p.play_cursor + cursor;
      } else {
        actual_cursor = p.play_cursor - cursor;
      }

      if (s->loop.mode == Loop_FORWARD && actual_cursor >= s->loop.end) {
        actual_cursor = s->loop.start + (actual_cursor - s->loop.end);
      }

      if (s->loop.mode == Loop_REVERSE && actual_cursor >= s->loop.end) {
        p.direction = !p.direction;
        actual_cursor = s->loop.end - (actual_cursor - cursor);
      }

      if (s->loop.mode == Loop_REVERSE && actual_cursor >= s->loop.end) {
        p.direction = !p.direction;
        actual_cursor = s->loop.end - (actual_cursor - s->loop.end);
      }

      if (s->loop.mode == Loop_REVERSE && p.direction == false && actual_cursor <= s->loop.start) {
        actual_cursor = s->loop.end - (s->loop.start - actual_cursor);
      }

      if (s->loop.mode == Loop_PINGPONG && actual_cursor >= s->loop.end) {
        p.direction = !p.direction;
        actual_cursor = s->loop.end - (actual_cursor - s->loop.end);
      }

      if (s->loop.mode == Loop_PINGPONG && p.direction == false && actual_cursor <= s->loop.start) {
        p.direction = !p.direction;
        actual_cursor = s->loop.start + (s->loop.start - actual_cursor);
      }

      if (actual_cursor >= wav_len) {
        break;
      }

      float valueL = buffer_sample[actual_cursor] * volume * left_vol;
      float valueR = buffer_sample[actual_cursor] * volume * right_vol;

      *(out++) += valueL;
      *(out++) += valueR;
    }

    if (actual_cursor >= wav_len) {
      p.set = false;
    } else {
      p.play_cursor = actual_cursor;
    }

    playbacks->list[i] = p;
  }
}

