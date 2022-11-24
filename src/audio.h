#ifndef __AUDIO_H__
#define __AUDIO_H__

#include <SDL.h>
#include <stdbool.h>
typedef enum LoopMode {

  Loop_OFF, 
  Loop_FORWARD,
  Loop_REVERSE,
  Loop_PINGPONG
} LoopMode;

typedef struct Loop {
  LoopMode mode;
  Uint32 start;
  Uint32 end;
} Loop;

typedef struct SampleDefinition {
  char name[256];
  char filename[256];
  Loop loop;
} SampleDefinition;

typedef struct Sample {
  char * name;
  char * filename;
  SDL_AudioSpec wav_spec;
  Uint32 wav_len;
  float * wav_buffer;
  short bit_depth;
  short size;
  Loop loop;
} Sample;

typedef struct SampleList {
  Sample list[512];
  short count;
} SampleList;

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
