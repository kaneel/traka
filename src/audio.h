#ifndef __AUDIO_H__
#define __AUDIO_H__

#include <SDL.h>
#include <stdbool.h>
#include "sample.h" 

typedef struct Playback {
  Uint32 play_cursor;
  bool direction;
  Sample * sample;
  short note;
  short volume;
  short octave;
  short channel;
  bool set;
} Playback;

typedef struct Channel {
  short volume;
  short panning;
} Channel;

typedef struct ChannelList {
  Channel list[64];
  short count;
} ChannelList;

typedef struct PlaybackList {
  Playback list[512];
  short count;
} PlaybackList;

typedef struct CallbackData {
  PlaybackList * playbacks;
  ChannelList * channels;
} CallbackData;

void audio_callback(void*, unsigned char *, int);

#endif
