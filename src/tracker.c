#include "SDL.h"
#include <stdbool.h>
#include <stdio.h>

#include "SDL_keycode.h"
#include "SDL_stdinc.h"
#include "app.h"
#include "theme.h"
#include "tracker.h"
#include "gui.h"

#define MAX_COL 2
#define MAX_ROW 16
#define COLUMN_WIDTH 110
#define COLUMN_MARGIN 10
#define ROW_HEIGHT 16
#define START_PATTERNS_X 80
#define START_PATTERNS_Y 140
#define START_PATTERNS_W 300
#define START_PATTERNS_H 200

const short NOTE_SET       = 0b100000;
const short OCTAVE_SET     = 0b010000;
const short INSTRUMENT_SET = 0b001000;
const short VOLUME_SET     = 0b000100;
const short FX_NUMBER_SET  = 0b000010;
const short FX_VALUE_SET   = 0b000001;

const short POSITIONS[9] = {
  /*
   * NOTES
   */
  0, 
  16,
  /* 
   * INSTRUMENT
   */
  30,
  38,
  /* 
   * VOLUME
   */
  50,
  58,
  /* 
   * EFFECT NUMBER
   */
  70,
  /* 
   * EFFECT VALUE
   */
  78,
  86,
};

static char * LETTERS[26] = {
  "A",
  "B",
  "C",
  "D",
  "E",
  "F",
  "G",
  "H",
  "I",
  "J",
  "K",
  "L",
  "M",
  "N",
  "O",
  "P",
  "Q",
  "R",
  "S",
  "T",
  "U",
  "V",
  "W",
  "X",
  "Y",
  "Z"
};

typedef char * Note;
static Note NOTES[15] = {
  [0]  = "--", 
  [1]  = "C",
  [2]  = "C",
  [3]  = "D",
  [4]  = "D",
  [5]  = "E",
  [6]  = "F",
  [7]  = "F",
  [8]  = "G",
  [9]  = "G",
  [10] = "A",
  [11] = "A",
  [12] = "B",
  [13] = "=",
  [14] = "^"
};

static char * NUMBERS[11] = {
  [0]  = "0",
  [1]  = "1",
  [2]  = "2",
  [3]  = "3",
  [4]  = "4",
  [5]  = "5",
  [6]  = "6",
  [7]  = "7",
  [8]  = "8",
  [9]  = "9",
};

static GUIElement GUISoundBox = {
  .state = (GUIElementState){
    .focus = false
  },
  .text = "Audio",
  .dimensions = { 500.0, 20.0, 60.0, 22.0 }
};

void move_prev_column(Cursor * currentCursor, bool from_tab) {
  PatternColumn * currentColumn = currentCursor->currentColumn;

  int prev = currentCursor->currentColumn->index - 1;
  currentCursor->currentColumn = currentCursor->parentPattern->columns[
    prev >= 0 ? prev : currentCursor->parentPattern->column_count - 1
  ];
  currentCursor->currentColumn->cursor[0] = from_tab ? 0 : 8;
  currentCursor->currentColumn->cursor[1] = currentColumn->cursor[1];
  currentColumn->cursor[0] = -1;
  currentColumn->cursor[1] = -1;
}

void move_next_column(Cursor * currentCursor) {
  PatternColumn * currentColumn = currentCursor->currentColumn;
  Pattern * currentPattern = currentCursor->parentPattern;

  int next = currentCursor->currentColumn->index + 1;
  currentCursor->currentColumn = currentPattern->columns[
    next > currentPattern->column_count - 1 ? 0 : next
  ];
  currentCursor->currentColumn->cursor[0] = 0;
  currentCursor->currentColumn->cursor[1] = currentColumn->cursor[1];
  currentColumn->cursor[0] = -1;
  currentColumn->cursor[1] = -1;
}

void tracker_keyboard(SDL_Keycode key, Cursor * currentCursor) {
  PatternColumn * currentColumn = currentCursor->currentColumn;
  short currentRow = currentCursor->currentColumn->cursor[1];
  short currentCursorX = currentCursor->currentColumn->cursor[0];
  bool moveRight = false;
  bool moveLeft = false;
  bool moveDown = false;
  bool moveUp = false;

  const Uint8 * kb_state = SDL_GetKeyboardState(NULL);

  switch(key) {
    case SDLK_KP_BACKSPACE: 
    case SDLK_BACKSPACE: 
      /*
       * THIS REMOVES A LINE AND GO UP
       */
      currentCursor->currentColumn->rows[currentRow] = (PatternRow){ 0, 0, 0, 0, 0, 0, 0};
      moveUp = true;
      break;
    case SDLK_DELETE: 
      /*
       * THIS REMOVES A LINE, stays at cursor and PULL other rows
       */
      for (short i = currentRow; i < currentColumn->row_len; i++) { 
        currentColumn->rows[i] = currentColumn->rows[i+1];
      }
      break;
    case SDLK_INSERT: 
      /*
       * INSERT A LINE
       */
      for (short i = currentColumn->row_len; i > currentRow; i--) {
        currentColumn->rows[i] = currentColumn->rows[i-1];
      } 
      currentColumn->rows[currentRow] = (PatternRow){ 0, 0, 0, 0, 0, 0, 0};
      break;
    case SDLK_PAGEDOWN:
      currentColumn->cursor[1] = currentColumn->cursor[1] + app_state.page_move >= currentColumn->row_len ? currentColumn->row_len - 1 : currentColumn->cursor[1] + app_state.page_move;
      break;
    case SDLK_PAGEUP:
      currentColumn->cursor[1] = currentColumn->cursor[1] - app_state.page_move < 0 ? 0 : currentColumn->cursor[1] - app_state.page_move;
      break;
    case SDLK_TAB: 
    case SDLK_KP_TAB: 
      /*
       * moves to the next column
       */
      if (kb_state[SDL_SCANCODE_LSHIFT]) {
        if (currentCursorX == 0) {
          move_prev_column(currentCursor, true);
        } else {
          currentCursor->currentColumn->cursor[0] = 0;
        }
      } else {
        move_next_column(currentCursor);
      }
      break;
    case SDLK_SPACE:
      if (currentCursorX == 0 || currentCursorX == 1) {
        currentColumn->rows[currentRow].note = app_state.last_note_used;
        currentColumn->rows[currentRow].octave = app_state.last_octave_used;
        currentColumn->rows[currentRow].instrument = app_state.last_instrument_used;
        currentColumn->rows[currentRow].set |= NOTE_SET | OCTAVE_SET | INSTRUMENT_SET;
      }

      if (currentCursorX == 2 || currentCursorX == 3) {
        currentColumn->rows[currentRow].instrument = app_state.last_instrument_used;
        currentColumn->rows[currentRow].set |= INSTRUMENT_SET;
      }

      if ((currentCursorX == 4 || currentCursorX == 5)) {
        currentColumn->rows[currentRow].volume = app_state.last_volume_used;
        currentColumn->rows[currentRow].set |= VOLUME_SET;
      }

      if (currentCursorX == 6) {
        currentColumn->rows[currentRow].effect_number = app_state.last_effect_number;
        currentColumn->rows[currentRow].set |= FX_NUMBER_SET;
      }

      if (currentCursorX == 7 || currentCursorX == 8) {
        currentColumn->rows[currentRow].effect_value = app_state.last_effect_value;
        currentColumn->rows[currentRow].set |= FX_VALUE_SET;
      }

      moveDown = true;
      break;
    default: 
      /*
       * THIS BIT is for notes, if you're 
       * on cursorX 0, you're entering a note
       */
      if (currentCursorX == 0) {
        int note = -1;
        switch(key) {
          case SDLK_MINUS:
          case SDLK_PERIOD:
            note = 0;
            moveDown = true;
            break;
          case SDLK_q: 
            note = 1;
            break;
          case SDLK_1:
            note = 2;
            break;
          case SDLK_w:
            note = 3;
            break;
          case SDLK_2:
            note = 4;
            break;
          case SDLK_e: 
            note = 5;
            break;
          case SDLK_r: 
            note = 6;
            break;
          case SDLK_5:
            note = 7;
            break;
          case SDLK_t:
            note = 8;
            break;
          case SDLK_6:
            note = 9;
            break;
          case SDLK_y:
            note = 10;
            break;
          case SDLK_7:
            note = 11;
            break;
          case SDLK_u:
            note = 12;
            break;
          case SDLK_EQUALS:
            // NOTE OFF
            if (kb_state[SDL_SCANCODE_LSHIFT]) {
              note = 14;
            } else {
              note = 13;
            }
            break;
          }

          if (note > -1) {
            moveDown = true;

            currentColumn->rows[currentRow].note = note;

            if (note < 13) {
              currentColumn->rows[currentRow].octave = app_state.last_octave_used;
              currentColumn->rows[currentRow].instrument = app_state.last_instrument_used;

              currentColumn->rows[currentRow].volume = app_state.last_volume_used;
              currentColumn->rows[currentRow].set |= 0b111000;
            }

            if (note == 0 || note == 13) {
              currentColumn->rows[currentRow].octave = 0;
              currentColumn->rows[currentRow].instrument = 0;
              currentColumn->rows[currentRow].volume = 0;
              currentColumn->rows[currentRow].set &= 0b000011;
            } else if (note == 14) {
              currentColumn->rows[currentRow].octave = 0;
              currentColumn->rows[currentRow].instrument = 0;
              currentColumn->rows[currentRow].volume = 0;
              currentColumn->rows[currentRow].effect_number = 0;
              currentColumn->rows[currentRow].effect_value = 0;
              currentColumn->rows[currentRow].set &= 0b000000;
            }

            app_state.last_note_used = note;
          }
        }

        if (currentCursorX == 6) {
          switch(key){
            default: {
              if (key >= 97 && key <= 122) {
                key -= 97;
                currentColumn->rows[currentRow].effect_number = key;
                currentColumn->rows[currentRow].set |= FX_NUMBER_SET;
                app_state.last_effect_number = key;
              }
              break;
            }
          }
        }

        /*
         * NOW, that's mostly for all numeric values
         * but cursor 6 is hexa.
         */
        if (currentCursorX == 1 ||
          currentCursorX == 2 ||
          currentCursorX == 3 ||
          currentCursorX == 4 ||
          currentCursorX == 5 ||
          currentCursorX == 7 ||
          currentCursorX == 8 
        ) {
          /* -2 not to catch UP/DOWN/LEFT/RIGTH 
           * -1 is for deletion 
           *  0 is 0 
           */
        int value = -2;
        switch(key) {
          case SDLK_MINUS:
          case SDLK_PERIOD:
            value = -1;
            moveDown = true;
            break;
          case SDLK_KP_1: 
          case SDLK_1: 
            value = 1;
            break;
          case SDLK_KP_2: 
          case SDLK_2:
            value = 2;
            break;
          case SDLK_KP_3: 
          case SDLK_3:
            value = 3;
            break;
          case SDLK_KP_4: 
          case SDLK_4:
            value = 4;
            break;
          case SDLK_KP_5: 
          case SDLK_5: 
            value = 5;
            break;
          case SDLK_KP_6: 
          case SDLK_6: 
            value = 6;
            break;
          case SDLK_KP_7: 
          case SDLK_7:
            value = 7;
            break;
          case SDLK_KP_8: 
          case SDLK_8:
            value = 8;
            break;
          case SDLK_KP_9: 
          case SDLK_9:
            value = 9;
            break;
          case SDLK_KP_0:
          case SDLK_0:
            value = 0;
            break;
          }

          if (value == -1) {
            if (currentCursorX == 1) {
              currentCursor->currentColumn->rows[currentRow].octave = 0;
              currentCursor->currentColumn->rows[currentRow].set ^= OCTAVE_SET;
            }
            if (currentCursorX == 2 || currentCursorX == 3) {
              currentCursor->currentColumn->rows[currentRow].instrument = 0;
              currentCursor->currentColumn->rows[currentRow].set ^= INSTRUMENT_SET;
            }
            if (currentCursorX == 4 || currentCursorX == 5) {
              currentCursor->currentColumn->rows[currentRow].volume = 0;
              currentCursor->currentColumn->rows[currentRow].set ^= VOLUME_SET;
            }
            if (currentCursorX == 6) {
              currentCursor->currentColumn->rows[currentRow].effect_number = 0;
              currentCursor->currentColumn->rows[currentRow].set ^= FX_NUMBER_SET;
            }
            if (currentCursorX == 7 || currentCursorX == 8) {
              currentCursor->currentColumn->rows[currentRow].effect_value = 0;
              currentCursor->currentColumn->rows[currentRow].set ^= FX_VALUE_SET;
            }
          } else if (value > -1) {
            if (currentCursorX == 2 || currentCursorX == 4 || currentCursorX == 7) {
              moveRight = true;
            } else if (currentCursorX == 3 || currentCursorX == 5 || currentCursorX == 8) {
              moveLeft = true;
              moveDown = true;
            } else if (currentCursorX == 6) {
              moveDown = true;
            }
            
            if (currentCursorX == 1) {
              currentCursor->currentColumn->rows[currentRow].octave = value;
              app_state.last_octave_used = value;
              currentCursor->currentColumn->rows[currentRow].set |= OCTAVE_SET;
            }

            if (currentCursorX == 2) {
              short curr = currentCursor->currentColumn->rows[currentRow].instrument % 10;
              short final = (value * 10) + curr;
              currentCursor->currentColumn->rows[currentRow].instrument = final;
              app_state.last_instrument_used = final;
              currentCursor->currentColumn->rows[currentRow].set |= INSTRUMENT_SET;
            }

            if (currentCursorX == 3) {
              short curr = floor(currentCursor->currentColumn->rows[currentRow].instrument / 10.0);
              short final = (curr * 10) + value;
              currentCursor->currentColumn->rows[currentRow].instrument = final;
              app_state.last_instrument_used = final;
              currentCursor->currentColumn->rows[currentRow].set |= INSTRUMENT_SET;
            }

            if (currentCursorX == 4) {
              int curr = currentCursor->currentColumn->rows[currentRow].volume % 10;
              short final = (value * 10) + curr;
              currentCursor->currentColumn->rows[currentRow].volume = final;
              app_state.last_volume_used = final;
              currentCursor->currentColumn->rows[currentRow].set |= VOLUME_SET;
            }

            if (currentCursorX == 5) {
              short curr = floor(currentCursor->currentColumn->rows[currentRow].volume / 10.0);
              short final = (curr * 10) + value;
              currentCursor->currentColumn->rows[currentRow].volume = final;
              app_state.last_volume_used = final;
              currentCursor->currentColumn->rows[currentRow].set |= VOLUME_SET;
            }

            if (currentCursorX == 7) {
              int curr = currentCursor->currentColumn->rows[currentRow].effect_value % 10;
              short final = (value * 10) + curr;
              currentCursor->currentColumn->rows[currentRow].effect_value = final;
              app_state.last_effect_value = final;
              currentCursor->currentColumn->rows[currentRow].set |= FX_VALUE_SET;
            }

            if (currentCursorX == 8) {
              short curr = floor(currentCursor->currentColumn->rows[currentRow].effect_value / 10.0);
              short final = (curr * 10) + value;
              currentCursor->currentColumn->rows[currentRow].effect_value = final;
              app_state.last_effect_value = final;
              currentCursor->currentColumn->rows[currentRow].set |= FX_VALUE_SET;
            }
          }
        }
    break;
  }

  if (key == SDLK_LEFT || moveLeft) {
    currentCursor->currentColumn->cursor[0] -= 1;

    if (currentCursor->currentColumn->cursor[0] < 0) {
      move_prev_column(currentCursor, false);
    }
  }

  if (key == SDLK_RIGHT || moveRight) {
    currentCursor->currentColumn->cursor[0] += 1;

    if (currentCursor->currentColumn->cursor[0] == 9) {
      move_next_column(currentCursor);
    }
  }

  if ((key == SDLK_UP || moveUp) && currentColumn->cursor[1] > 0) {
    currentCursor->currentColumn->cursor[1] -= 1;
  }

  if ((key == SDLK_DOWN || moveDown) && currentColumn->cursor[1] < currentColumn->row_len - 1) {
    currentCursor->currentColumn->cursor[1] += 1;
  }
}

short find_playback_index(Playback * list, short max) {
  short i = 0;
  for (i = 0; i < max; i++) {
    if (list[i].set == false) {
      break;
    }
  }
  return i;
}

void stop_channel_playbacks(PlaybackList * playbacks, short channel) {
  for (short i = 0; i < playbacks->count; i++) {
    if (playbacks->list[i].set && playbacks->list[i].channel == channel) {
      playbacks->list[i].set = false;
    }
  }
}

void check_row(short row, Pattern * pattern, PlaybackList * playbacks, ChannelList * channels, SampleList * samples) {
  for (int i = 0; i < pattern->column_count; i++) {
    PatternColumn * col = pattern->columns[i];
    short actual_row = row % col->row_len;
    PatternRow row = col->rows[actual_row];

    if ((row.set&(NOTE_SET | OCTAVE_SET | INSTRUMENT_SET)) == (NOTE_SET | OCTAVE_SET | INSTRUMENT_SET )) {
      short index = find_playback_index(playbacks->list, playbacks->count);

      if (index == playbacks->count) {
        playbacks->count = index + 1;
      }

      Playback playback = {
        .note = row.note,
        .octave = row.octave,
        .volume = (row.set & VOLUME_SET) == VOLUME_SET? row.volume : 64,
        .sample = &samples->list[row.instrument],
        .play_cursor = 0,
        .channel = i,
        .set = true,
        .direction = true
      };
      playbacks->list[index] = playback;
    } else if (row.note > 13) {
      stop_channel_playbacks(playbacks, i);
    }

    if ((row.set&(FX_NUMBER_SET | FX_VALUE_SET)) == (FX_NUMBER_SET | FX_VALUE_SET)) {
      switch(row.effect_number) {
        case 12: /* M */ 
          channels->list[i].volume = MIN(row.effect_value, 80);
          break;
        case 15: /* P */ 
          channels->list[i].panning = MIN(row.effect_value, 80);
          break;
      }
    }
  }
}

void draw_row(PatternRow row, Cursor cursor, short startX, short startY, short corrected_col, short corrected_row, bool is_ghost) {
  nvgBeginPath(app.nvg); 

  nvgFontFace(app.nvg, "black");
  nvgFontSize(app.nvg, 16.0f);
  nvgTextAlign(app.nvg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);

  int x = START_PATTERNS_X + (COLUMN_WIDTH * corrected_col);
  int y = START_PATTERNS_Y + (ROW_HEIGHT * corrected_row);

  if((corrected_row + startY) % app_state.lpb == 0) {
    nvgFillColor(app.nvg, theme.beatHighlight);
    nvgRect(app.nvg, x, y, 94, ROW_HEIGHT);
    nvgFill(app.nvg);
  }

  NVGcolor textColour = is_ghost ? theme.ghostColour : theme.textColour;

  nvgFillColor(app.nvg, 
    cursor.currentColumn->index == corrected_col + startX
      && cursor.currentColumn->cursor[0] == 0
      && cursor.currentColumn->cursor[1] == corrected_row + startY
        ? theme.cursorTextColour
        : textColour
      );

  short note = row.note;
  if (note == 13) {
    // DRAW NOTE OFF 
    nvgText(app.nvg, x, y, "=", NULL);
    nvgFillColor(app.nvg, textColour);
    nvgText(app.nvg, x + 8, y, "=", NULL);
    nvgText(app.nvg, x + 16, y, "=", NULL);
  } else if (note == 14) {
    // DRAW NOTE CUT 
    nvgText(app.nvg, x, y, "^", NULL);
    nvgFillColor(app.nvg, textColour);
    nvgText(app.nvg, x + 8, y, "^", NULL);
    nvgText(app.nvg, x + 16, y, "^", NULL);
  } else if (note == 2 ||
      note == 4 || 
      note == 7 ||
      note == 9 || 
      note == 11) {
    // INCIDENTALS!
    // draw note - 1, so C# is C, D# is D…
    nvgText(app.nvg, x, y, NOTES[note - 1] , NULL);
    nvgFillColor(app.nvg, textColour);
    // and adds the incidental after
    nvgText(app.nvg, x + 8, y, "#", NULL);
  } else if ((row.set & NOTE_SET) == NOTE_SET){
    // notes, just notes
    nvgText(app.nvg, x, y, NOTES[note] , NULL);
    nvgFillColor(app.nvg, textColour);
    // no incidentals
    nvgText(app.nvg, x + 8, y, "-", NULL);
  } else {
    // notes, just notes
    nvgText(app.nvg, x, y, "-", NULL);
    nvgFillColor(app.nvg, textColour);
    // no incidentals
    nvgText(app.nvg, x + 8, y, "-", NULL);
  }

  for(short i = 1; i < 9; i++) {
    nvgFillColor(app.nvg, 
      cursor.currentColumn->index == corrected_col + startX
        && cursor.currentColumn->cursor[0] == i
        && cursor.currentColumn->cursor[1] == corrected_row + startY
          ? theme.cursorTextColour
          : textColour
        );

    short value;
    short full;
    short bit_val;

    if (i == 1) {
      bit_val = 4;
      value = row.octave;
    } else if (i == 2) {
      bit_val = 3;
      full = row.instrument;
      value = floor(full / 10.0);
    } else if (i == 3) {
      bit_val = 3;
      full = row.instrument;
      value = full % 10;
    } else if (i == 4) {
      bit_val = 2;
      full = row.volume;
      value = floor(full / 10.0);
    } else if (i == 5) {
      bit_val = 2;
      full = row.volume;
      value = full % 10;
    } else if (i == 6) {
      bit_val = 1;
      full = row.effect_number;
      value = full;
    } else if (i == 7) {
      bit_val = 0;
      full = row.effect_value;
      value = floor(full / 10.0);
    } else {
      bit_val = 0;
      full = row.effect_value;
      value = full % 10;
    }

    if ((row.set & (0b1<<bit_val)) == (0b1<<bit_val)) {
      if (i == 6)
        nvgText(app.nvg, x + POSITIONS[i], y, LETTERS[value], NULL);
      else
        nvgText(app.nvg, x + POSITIONS[i], y, NUMBERS[value], NULL);
    } else if (i > 1 || note < 13) {
      nvgText(app.nvg, x + POSITIONS[i], y, "-", NULL);
    }
  }
}

void draw_play_heads(Pattern * pat, Uint32 row) {
  short start = app_state.visible_pattern_column;

  for (short i = start; i < start + MAX_COL; i++) {
    short corrected_col = i - start;
    PatternColumn * column = pat->columns[i];
    unsigned int real_row = row % column->row_len;

    if (
         real_row >= (Uint32)app_state.visible_pattern_row + MAX_ROW
      || real_row < (Uint32)app_state.visible_pattern_row
    ) return;

    unsigned int corrected_row = real_row - app_state.visible_pattern_row;

    nvgBeginPath(app.nvg); 
    nvgFillColor(app.nvg, theme.playHeadColour);
    nvgRect(app.nvg, (START_PATTERNS_X - 12) + (COLUMN_WIDTH * corrected_col), START_PATTERNS_Y + (corrected_row * ROW_HEIGHT), 8, ROW_HEIGHT);
    nvgFill(app.nvg);
  }
}

void draw_pattern(Pattern * pat, Cursor cursor) {
  if (cursor.currentColumn->index >= app_state.visible_pattern_column + MAX_COL) {
    app_state.visible_pattern_column = cursor.currentColumn->index - (MAX_COL - 1);
  } else if (cursor.currentColumn->index < app_state.visible_pattern_column) {
    app_state.visible_pattern_column = cursor.currentColumn->index;
  }

  if (cursor.currentColumn->cursor[1] >= app_state.visible_pattern_row + MAX_ROW) {
    app_state.visible_pattern_row = cursor.currentColumn->cursor[1] - (MAX_ROW - 1);
  } else if (cursor.currentColumn->cursor[1] < app_state.visible_pattern_row) {
    app_state.visible_pattern_row = cursor.currentColumn->cursor[1];
  }

  short startX = app_state.visible_pattern_column;
  short startY = app_state.visible_pattern_row;

  for (short i = startX; i < startX + MAX_COL; i++) {
    short corrected_col = i - startX;
    PatternColumn * col = pat->columns[i];

    nvgBeginPath(app.nvg); 
    nvgFillColor(app.nvg, theme.textColour);
    nvgFontSize(app.nvg, 16.0f);
    nvgFontFace(app.nvg, "thin");
    nvgTextAlign(app.nvg, NVG_ALIGN_MIDDLE | NVG_ALIGN_CENTER);
    char text[10];
    snprintf(text, 10, "CHANNEL %d", i + 1);
    nvgTextBox(app.nvg, START_PATTERNS_X +  (COLUMN_WIDTH * corrected_col), START_PATTERNS_Y - 38, COLUMN_WIDTH - 16, text, NULL);

    if (col->cursor[0] > -1 && col->cursor[1] > -1) {
      nvgBeginPath(app.nvg); 
      nvgFillColor(app.nvg, theme.cursorBackground);
      nvgRect(app.nvg, START_PATTERNS_X + (COLUMN_WIDTH * corrected_col) + POSITIONS[col->cursor[0]], START_PATTERNS_Y + ((col->cursor[1] - startY) * ROW_HEIGHT), 8, ROW_HEIGHT);
      nvgFill(app.nvg);
    }

    for (int k = startY; k < startY + MAX_ROW; k++) {
      draw_row(col->rows[k % col->row_len], cursor, startX, startY, corrected_col, k - startY, k >= col->row_len);
    }
  }
}

void draw_tracker_ui(CallbackData * callbackData) {
  int x = 20.0;
  int y = 20.0;

  if (GUI_Button((GUIElement){
    .dimensions = { x, y, 60.0, 22.0},    
    .text = "play",
    .state = { 0, 0, 0 }
  }, app_state.is_playing)) {
    app_state.is_playing = true;
  }

  if (GUI_Button((GUIElement){
    .dimensions = { x += 60.0, y, 60.0, 22.0},    
    .text = "stop",
    .state = { 0, 0, 0 }
  }, !app_state.is_playing)) {
    app_state.is_playing = false;
  }

  // BLOCK BPM
  x = 200.0;
  if (GUI_Button((GUIElement){
    .dimensions = { x, y, 10.0, 22.0 },
    .text = "-",
    .state = { 0, 0, 0 }
  }, false)) {
    if (app_state.bpm > 1) {
      app_state.bpm -= 1;
    }
  }
  GUI_Input((GUIElement){
    .dimensions = { x += 10.0, y, 40.0, 22.0},    
    .numericValue = app_state.bpm
  }, false);
  if (GUI_Button((GUIElement){
    .dimensions = { x +40, y, 10.0, 22.0},    
    .text = "+"
  }, false)) {
    if (app_state.bpm < 512) {
      app_state.bpm += 1;
    }
  }

  x = 300.0;
  if (GUI_Button((GUIElement){
    .dimensions = { x, y, 10.0, 22.0},    
    .text = "-"
  }, false)) {
    if (app_state.lpb > 1) {
      app_state.lpb -= 1;
    }
  }
  GUI_Input((GUIElement){
    .dimensions = { x += 10.0, y, 40.0, 22.0},    
    .numericValue = app_state.lpb
  },false);
  if (GUI_Button((GUIElement){
    .dimensions = { x += 40.0, y, 10.0, 22.0},    
    .text = "+"
  }, false)) {
    if (app_state.lpb < 64) {
      app_state.lpb += 1;
    }
  }

  x = 400.0;
  if (GUI_Button((GUIElement){
    .dimensions = { x, y, 10.0, 22.0},    
    .text = "-"
  }, false)) {
    if (app_state.last_instrument_used > 0) {
      app_state.last_instrument_used -= 1;
    }
  }
  GUI_Input((GUIElement){
    .dimensions = { x += 10.0, y, 40.0, 22.0},    
    .numericValue = app_state.last_instrument_used
  }, false);
  if (GUI_Button((GUIElement){
    .dimensions = { x += 40.0, y, 10.0, 22.0},    
    .text = "+"
  }, false)) {
    if (app_state.last_instrument_used < 99) {
      app_state.last_instrument_used += 1;
    }
  }

  x = 500.0;
  if (GUI_ComboBox(GUISoundBox, false)) {
    GUISoundBox.state.focus = !GUISoundBox.state.focus;
  }

  if (GUISoundBox.state.focus) {
    nvgBeginPath(app.nvg); 
    nvgBeginPath(app.nvg); 
    nvgRect(app.nvg, GUISoundBox.dimensions[0], GUISoundBox.dimensions[1] + 22.0, 200, 100);
    nvgStrokeWidth(app.nvg, 2.f);
    nvgStrokeColor(app.nvg, nvgRGBAf(.0f, .0f, .0f, 1.0f));
    nvgStroke(app.nvg);
    nvgFillColor(app.nvg, nvgRGBAf(.9f, .9f, .9f, 1.0f));
    nvgFill(app.nvg);

    nvgFillColor(app.nvg, theme.textColour);
    unsigned short count = SDL_GetNumAudioDevices(0);

    for (unsigned short i = 0; i < count; ++i) {
      if (GUI_TextButton(( GUIElement ) {
        .text = (char *)SDL_GetAudioDeviceName(i, 0),
        .dimensions = { x, 52 + (22 * i), 0, 0 }
      }, app_state.audio_device_id == i)) {
        app_state.audio_device_id = i;
        SDL_PauseAudioDevice(app.audio_device_id, 1);
        set_audio_device(app_state.audio_device_id, audio_callback, callbackData);

        if (app_state.is_playing) {
          SDL_PauseAudioDevice(app.audio_device_id, 0);
        }
      }
    }
  }
}

void draw_pattern_view(CallbackData * callbackData, Pattern * pattern, Cursor currentCursor) {
  draw_tracker_ui(callbackData);

  int zone_x = START_PATTERNS_X;
  int zone_y = START_PATTERNS_Y;
  int zone_w = (COLUMN_WIDTH * MAX_COL) - 20;
  int zone_h = (ROW_HEIGHT * MAX_ROW);

  nvgBeginPath(app.nvg);

  // background
  nvgStrokeColor(app.nvg, nvgRGBAf(0.5f, 0.f, 0.f, 1.0f));
  nvgFillColor(app.nvg, nvgRGBAf(0.9f, 0.9f, 0.9f, 1.0f));
  nvgRect(app.nvg, zone_x - 20, zone_y - 50, zone_w + 40, zone_h + 60);
  nvgFill(app.nvg);
  nvgStroke(app.nvg);

  draw_pattern(pattern, currentCursor);

  if (app_state.is_playing && app_state.song_position >= 0) {
    draw_play_heads(pattern, app_state.song_position);
  }
}
