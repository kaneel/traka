#ifndef __TRACKER_H__
#define __TRACKER_H__

#include <SDL.h>
#include <stdbool.h>

#include "audio.h"
#include "sample.h"

typedef struct PatternRow {
  short note;
  short octave;
  short instrument;
  short volume;
  short effect_number;
  short effect_value;
  short set;
} PatternRow;

typedef struct PatternColumn {
  PatternRow rows[1024];
  short row_len;
  short cursor[2];
  short index;
} PatternColumn;

typedef struct Pattern {
  PatternColumn * columns[512];
  short column_count;
} Pattern;

typedef struct Track {
  Pattern * patterns[9999];
  short pattern_count;
} Track;

typedef struct Cursor {
  PatternColumn * currentColumn;
  Pattern * parentPattern;
} Cursor;

void move_prev_column(Cursor *, bool);
void move_next_column(Cursor *);
void tracker_keyboard(SDL_Keycode, Cursor *);
short find_playback_index(Playback *, short);
void stop_channel_playbacks(PlaybackList *, short);
void check_row(short, Pattern *, PlaybackList *, ChannelList *, SampleList *);
void draw_row(PatternRow, Cursor, short, short, short, short, bool);
void draw_play_heads(Pattern *, Uint32);
void draw_pattern(Pattern *, Cursor);
void draw_pattern_view(CallbackData *, Pattern *, Cursor);

#endif
