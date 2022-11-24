#ifndef __SAMPLE_H__
#define __SAMPLE_H__

#include "SDL_keycode.h"
#include "gui.h"
#include "audio.h"
#include <SDL.h>

Sample load_sample(char [256], char [256], char [256]);
void load_samples(SampleList *, SampleDefinition *, short);
void samples_keyboard(SDL_Keycode);
void draw_sample(Sample *, PlaybackList *);
void draw_sample_list(SampleList *);
void draw_sample_view(SampleList *, PlaybackList *);
void draw_sample_ui(Sample *);

#endif
