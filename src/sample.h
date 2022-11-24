#ifndef __SAMPLE_H__
#define __SAMPLE_H__

#include "SDL_keycode.h"
#include "gui.h"
#include <SDL.h>

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

typedef struct SampleDefinition {
  char name[256];
  char filename[256];
  Loop loop;
} SampleDefinition;

Sample load_sample(char [256], char [256], char [256]);
void load_samples(SampleList *, SampleDefinition *, short);
void samples_keyboard(SDL_Keycode);
void draw_sample(Sample *);
void draw_sample_list(SampleList *);
void draw_sample_view(SampleList *);
void draw_sample_ui(Sample *);

#endif
