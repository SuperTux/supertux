//
// C Implementation: globals
//
// Description:
//
//
// Author: Tobias Glaesser <tobi.web@gmx.de>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "globals.h"

/** The datadir prefix prepended when loading game data file */
std::string datadir;

SDL_Surface * screen;
text_type black_text, gold_text, blue_text, red_text, yellow_nums, white_text, white_small_text, white_big_text;

MouseCursor * mouse_cursor;

bool use_gl;
bool use_joystick;
bool use_fullscreen;
bool debug_mode;
bool show_fps;

int joystick_num = 0;
char* level_startup_file = 0;
bool launch_worldmap_mode = false;

/* SuperTux directory ($HOME/.supertux) and save directory($HOME/.supertux/save) */
char *st_dir, *st_save_dir;

SDL_Joystick * js;

/* Returns 1 for every button event, 2 for a quit event and 0 for no event. */
int wait_for_event(SDL_Event& event,unsigned int min_delay, unsigned int max_delay, bool empty_events)
{
  int i;
  timer_type maxdelay;
  timer_type mindelay;
  timer_init(&maxdelay,false);
  timer_init(&mindelay,false);

  if(max_delay < min_delay)
    max_delay = min_delay;

  timer_start(&maxdelay,max_delay);
  timer_start(&mindelay,min_delay);

  if(empty_events)
    while (SDL_PollEvent(&event))
    {}

  /* Handle events: */

  for(i = 0; timer_check(&maxdelay) || !i; ++i)
    {
      while (SDL_PollEvent(&event))
        {
          if(!timer_check(&mindelay))
            {
              if (event.type == SDL_QUIT)
                {
                  /* Quit event - quit: */
                  return 2;
                }
              else if (event.type == SDL_KEYDOWN)
                {
                  /* Keypress - skip intro: */

                  return 1;
                }
              else if (event.type == SDL_JOYBUTTONDOWN)
                {
                  /* Fire button - skip intro: */

                  return 1;
                }
              else if (event.type == SDL_MOUSEBUTTONDOWN)
                {
                  /* Mouse button - skip intro: */
                  return 1;
                }
            }
        }
      SDL_Delay(10);
    }

  return 0;
}
