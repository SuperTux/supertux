
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
  
/*  December 28, 2003 - December 30, 2003 */
 
/* leveleditor.c - A built-in level editor for SuperTux
 by Ricardo Cruz <rick2@aeiou.pt>                      */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <SDL.h>
#include <SDL_image.h>

#include "leveleditor.h"
#include "gameloop.h"
#include "screen.h"
#include "defines.h"
#include "globals.h"
#include "setup.h"
#include "menu.h"
#include "level.h"
#include "badguy.h"

/* definitions to aid development */
#define DONE_LEVELEDITOR    1
#define DONE_QUIT        2
#define DONE_CHANGELEVEL  3

/* definitions that affect gameplay */
#define KEY_CURSOR_SPEED 32
#define KEY_CURSOR_FASTSPEED 64
#define KEY_LEFT_MARGIN 160
#define KEY_RIGHT_MARGIN 480

#define MOUSE_LEFT_MARGIN 32
#define MOUSE_RIGHT_MARGIN 608
#define MOUSE_POS_SPEED 32

  /* Level Intro: */
/*
  clearscreen(0, 0, 0);

  sprintf(str, "Editing Level %s", levelfilename);
  drawcenteredtext(str, 200, letters_red, NO_UPDATE, 1);

  sprintf(str, "%s", levelname);
  drawcenteredtext(str, 224, letters_gold, NO_UPDATE, 1);

  SDL_Flip(screen);

  SDL_Delay(1000);
*/

/* global variables (based on the gameloop ones) */

bad_guy_type bad_guys[NUM_BAD_GUYS];
SDL_Surface *img_bsod_left[4], *img_laptop_left[3], *img_money_left[2];
int level;
st_level current_level;
char level_subset[100];

int frame;
SDL_Surface *selection;

/* gameloop funcs declerations */

void loadlevelgfx(void);
void unloadlevelgfx(void);
void add_bad_guy(int x, int y, int kind);
void loadshared(void);
void unloadshared(void);
void drawshape(int x, int y, unsigned char c);

/* own declerations */

void savelevel();
void le_loadlevel(void);
void le_change(int x, int y, int sx, unsigned char c);
void showhelp();
void le_set_defaults(void);
void le_activate_bad_guys(void);

void le_activate_bad_guys(void)
{
int x,y;

  /* Activate bad guys: */

  /* as oposed to the gameloop.c func, this one doesn't remove
  the badguys from tiles                                    */

  for (y = 0; y < 15; ++y)
    for (x = 0; x < current_level.width; ++x)
      if (current_level.tiles[y][x] >= '0' && current_level.tiles[y][x] <= '9')
        add_bad_guy(x * 32, y * 32, current_level.tiles[y][x] - '0');
}

void le_set_defaults()
{
  /* Set defaults: */

  if(current_level.time_left == 0)
    current_level.time_left = 255;
}

/* FIXME: Needs to be implemented. It should ask the user for the level(file)name and then let him create a new level based on this. */
void newlevel()
{
}

/* FIXME: It should let select the user a level, which is in the leveldirectory and then load it. */
void selectlevel()
{
}

int leveleditor()
{
  char str[10];
  int done;
  int x, y, i;	/* for cicles */
  int pos_x, cursor_x, cursor_y, old_cursor_x, fire;
  SDL_Event event;
  SDLKey key;
  SDLMod keymod;
  int last_time, now_time;
  
  strcpy(level_subset,"default");

  level = 1;

  initmenu();
  menumenu = MENU_LEVELEDITOR;
  show_menu = YES;

  frame = 0;	/* support for frames in some tiles, like waves and bad guys */
  
  loadshared();
  loadlevel(&current_level,"default",level);
  loadlevelgfx();

  le_activate_bad_guys();
  le_set_defaults();
  
  selection = load_image(DATA_PREFIX "/images/leveleditor/select.png", USE_ALPHA);

  done = 0;
  pos_x = 0;
  cursor_x = 3*32;
  old_cursor_x = cursor_x;
  cursor_y = 2*32;
  fire = DOWN;

  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

  while(1)
    {
      SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, current_level.bkgd_red, current_level.bkgd_green, current_level.bkgd_blue));

      last_time = SDL_GetTicks();
      frame++;
      
      keymod = SDL_GetModState();

      while(SDL_PollEvent(&event))
        {
          // testing SDL_KEYDOWN, SDL_KEYUP and SDL_QUIT events
          switch(event.type)
            {
            case SDL_KEYDOWN:	// key pressed
              key = event.key.keysym.sym;
              if(show_menu)
                {
                  menu_event(key);
                  break;
                }
              switch(key)
                {
                case SDLK_LEFT:
                  if(fire == DOWN)
                    cursor_x -= KEY_CURSOR_SPEED;
                  else
                    cursor_x -= KEY_CURSOR_FASTSPEED;

                  if(cursor_x < 0)
                    cursor_x = 0;
                  break;
                case SDLK_RIGHT:
                  if(fire == DOWN)
                    cursor_x += KEY_CURSOR_SPEED;
                  else
                    cursor_x += KEY_CURSOR_FASTSPEED;

                  if(cursor_x > (current_level.width*32) - 1)
                    cursor_x = (current_level.width*32) - 1;
                  break;
                case SDLK_UP:
                  if(fire == DOWN)
                    cursor_y -= KEY_CURSOR_SPEED;
                  else
                    cursor_y -= KEY_CURSOR_FASTSPEED;

                  if(cursor_y < 0)
                    cursor_y = 0;
                  break;
                case SDLK_DOWN:
                  if(fire == DOWN)
                    cursor_y += KEY_CURSOR_SPEED;
                  else
                    cursor_y += KEY_CURSOR_FASTSPEED;

                  if(cursor_y > 480-32)
                    cursor_y = 480-32;
                  break;
                case SDLK_LCTRL:
                  fire = UP;
                  break;
                case SDLK_F1:
                  showhelp();
                  break;
                case SDLK_HOME:
                  cursor_x = 0;
                  break;
                case SDLK_END:
                  cursor_x = (current_level.width * 32) - 32;
                  break;
                case SDLK_PERIOD:
                  le_change(cursor_x, cursor_y, 0, '.');
                  break;
                case SDLK_a:
                  if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
                    le_change(cursor_x, cursor_y, 0, 'A');
                  else
                    le_change(cursor_x, cursor_y, 0, 'a');
                  break;
                case SDLK_b:
                  if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
                    le_change(cursor_x, cursor_y, 0, 'B');
                  break;
                case SDLK_c:
                  if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
                    le_change(cursor_x, cursor_y, 0, 'C');
                  else
                    le_change(cursor_x, cursor_y, 0, 'c');
                  break;
                case SDLK_d:
                  if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
                    le_change(cursor_x, cursor_y, 0, 'D');
                  else
                    le_change(cursor_x, cursor_y, 0, 'd');
                  break;
                case SDLK_e:
                  if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
                    le_change(cursor_x, cursor_y, 0, 'E');
                  else
                    le_change(cursor_x, cursor_y, 0, 'e');
                  break;
                case SDLK_f:
                  if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
                    le_change(cursor_x, cursor_y, 0, 'F');
                  else
                    le_change(cursor_x, cursor_y, 0, 'f');
                  break;
                case SDLK_g:
                  if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
                    le_change(cursor_x, cursor_y, 0, 'G');
                  else
                    le_change(cursor_x, cursor_y, 0, 'g');
                  break;
                case SDLK_h:
                  if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
                    le_change(cursor_x, cursor_y, 0, 'H');
                  else
                    le_change(cursor_x, cursor_y, 0, 'h');
                  break;
                case SDLK_i:
                  if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
                    le_change(cursor_x, cursor_y, 0, 'I');
                  else
                    le_change(cursor_x, cursor_y, 0, 'i');
                  break;
                case SDLK_j:
                  if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
                    le_change(cursor_x, cursor_y, 0, 'J');
                  else
                    le_change(cursor_x, cursor_y, 0, 'j');
                  break;
                case SDLK_x:
                  if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
                    le_change(cursor_x, cursor_y, 0, 'X');
                  else
                    le_change(cursor_x, cursor_y, 0, 'x');
                  break;
                case SDLK_y:
                  if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
                    le_change(cursor_x, cursor_y, 0, 'Y');
                  else
                    le_change(cursor_x, cursor_y, 0, 'y');
                  break;
                case SDLK_LEFTBRACKET:
                  le_change(cursor_x, cursor_y, 0, '[');
                  break;
                case SDLK_RIGHTBRACKET:
                  le_change(cursor_x, cursor_y, 0, ']');
                  break;
                case SDLK_HASH:
                case SDLK_3:
                  if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
                    le_change(cursor_x, cursor_y, 0, '#');
                  break;
                case SDLK_DOLLAR:
                case SDLK_4:
                  if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
                    le_change(cursor_x, cursor_y, 0, '$');
                  break;
                case SDLK_BACKSLASH:
                  if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
                    le_change(cursor_x, cursor_y, 0, '|');
                  else
                    le_change(cursor_x, cursor_y, 0, '\\');
                  break;
                case SDLK_CARET:
                  le_change(cursor_x, cursor_y, 0, '^');
                  break;
                case SDLK_AMPERSAND:
                case SDLK_6:
                  if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
                    le_change(cursor_x, cursor_y, 0, '&');
                  break;
                case SDLK_EQUALS:
                case SDLK_0:
                  if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
                    le_change(cursor_x, cursor_y, 0, '=');
                  else		/* let's add a bad guy */
                    le_change(cursor_x, cursor_y, 0, '0');

                  for(i = 0; i < NUM_BAD_GUYS; ++i)
                    if (bad_guys[i].alive == NO)
                      {
                        bad_guys[i].alive = YES;
                        bad_guys[i].kind = BAD_BSOD;
                        bad_guys[i].x = (((int)cursor_x/32)*32);
                        bad_guys[i].y = (((int)cursor_y/32)*32);
                        i = NUM_BAD_GUYS;
                      }
                  break;
                case SDLK_1:
                  if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
                    le_change(cursor_x, cursor_y, 0, '!');
                  else		/* let's add a bad guy */
                    le_change(cursor_x, cursor_y, 0, '1');

                  for(i = 0; i < NUM_BAD_GUYS; ++i)
                    if (bad_guys[i].alive == NO)
                      {
                        bad_guys[i].alive = YES;
                        bad_guys[i].kind = BAD_LAPTOP;
                        bad_guys[i].x = (((int)cursor_x/32)*32);
                        bad_guys[i].y = (((int)cursor_y/32)*32);
                        i = NUM_BAD_GUYS;
                      }
                  break;
                case SDLK_2:
                  le_change(cursor_x, cursor_y, 0, '2');

                  for(i = 0; i < NUM_BAD_GUYS; ++i)
                    if (bad_guys[i].alive == NO)
                      {
                        bad_guys[i].alive = YES;
                        bad_guys[i].kind = BAD_MONEY;
                        bad_guys[i].x = (((int)cursor_x/32)*32);
                        bad_guys[i].y = (((int)cursor_y/32)*32);
                        i = NUM_BAD_GUYS;
                      }
                  break;
                case SDLK_PLUS:
                  if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
                    le_change(cursor_x, cursor_y, 0, '*');
                  break;
                default:
                  break;
                }
              break;
            case SDL_KEYUP:	// key released
              switch(event.key.keysym.sym)
                {
                case SDLK_LCTRL:
                  fire = DOWN;
                  break;
                case SDLK_ESCAPE:
                  if(!show_menu)
                    show_menu = YES;
                  else
                    show_menu = NO;
                  break;
                default:
                  break;
                }
              break;
            case SDL_MOUSEBUTTONDOWN:
              if(event.button.button == SDL_BUTTON_LEFT)
                {
                  x = event.motion.x;
                  y = event.motion.y;

                  cursor_x = ((int)(pos_x + x) / 32) * 32;
                  cursor_y = ((int) y / 32) * 32;
                }
              break;
            case SDL_MOUSEMOTION:
              x = event.motion.x;
              y = event.motion.y;

              cursor_x = ((int)(pos_x + x) / 32) * 32;
              cursor_y = ((int) y / 32) * 32;
              break;
            case SDL_QUIT:	// window closed
              done = DONE_QUIT;
              break;
            default:
              break;
            }
        }

      /* mouse movements */
      x = event.motion.x;
      if(x < MOUSE_LEFT_MARGIN)
        pos_x -= MOUSE_POS_SPEED;
      else if(x > MOUSE_RIGHT_MARGIN)
        pos_x += MOUSE_POS_SPEED;

      if(old_cursor_x != cursor_x)
        {
          if(cursor_x < pos_x + KEY_LEFT_MARGIN)
            pos_x = cursor_x - KEY_LEFT_MARGIN;

          if(cursor_x > pos_x + KEY_RIGHT_MARGIN)
            pos_x = cursor_x - KEY_RIGHT_MARGIN;
        }

      if(pos_x < 0)
        pos_x = 0;
      if(pos_x > (current_level.width * 32) - 640)
        pos_x = (current_level.width * 32) - 640;

      old_cursor_x = cursor_x;

      for (y = 0; y < 15; ++y)
        for (x = 0; x < 21; ++x)
          drawshape(x * 32, y * 32, current_level.tiles[y][x + (pos_x / 32)]);

      /* Draw the Bad guys: */
      for (i = 0; i < NUM_BAD_GUYS; ++i)
        {
          /* printf("\nbad_guys[%i].alive = %i", i, bad_guys[i].alive); */
          if(bad_guys[i].alive == NO)
            continue;
          /* to support frames: img_bsod_left[(frame / 5) % 4] */
          if(bad_guys[i].kind == BAD_BSOD)
            drawimage(img_bsod_left[(frame / 5) % 4], ((int)(bad_guys[i].x - pos_x)/32)*32, bad_guys[i].y, NO_UPDATE);
          else if(bad_guys[i].kind == BAD_LAPTOP)
            drawimage(img_laptop_left[(frame / 5) % 3], ((int)(bad_guys[i].x - pos_x)/32)*32, bad_guys[i].y, NO_UPDATE);
          else if (bad_guys[i].kind == BAD_MONEY)
            drawimage(img_money_left[(frame / 5) % 2], ((int)(bad_guys[i].x - pos_x)/32)*32, bad_guys[i].y, NO_UPDATE);
        }


      drawimage(selection, ((int)(cursor_x - pos_x)/32)*32, cursor_y, NO_UPDATE);

      sprintf(str, "%d", current_level.time_left);
      drawtext("TIME", 324, 0, letters_blue, NO_UPDATE, 1);
      drawtext(str, 404, 0, letters_gold, NO_UPDATE, 1);

      sprintf(str, "%s", current_level.name);
      drawtext("NAME", 0, 0, letters_blue, NO_UPDATE, 1);
      drawtext(str, 80, 0, letters_gold, NO_UPDATE, 1);

      drawtext("F1 for Help", 10, 430, letters_blue, NO_UPDATE, 1);

      if(show_menu)
        {
          done = drawmenu();
          if(done)
            return 0;
        }
      if(done == DONE_QUIT)
        return 1;

      SDL_Delay(50);
	now_time = SDL_GetTicks();
	if (now_time < last_time + FPS)
		SDL_Delay(last_time + FPS - now_time);	/* delay some time */

      SDL_Flip(screen);
    }

  unloadlevelgfx();
  unloadshared();

  SDL_FreeSurface(selection);

  return done;
}

void le_change(int x, int y, int sx, unsigned char c)
{
  int xx, yy;
  int i;

  yy = (y / 32);
  xx = ((x + sx) / 32);

  /* if there is a bad guy over there, remove it */
  for(i = 0; i < NUM_BAD_GUYS; ++i)
    if (bad_guys[i].alive)
      if(xx == bad_guys[i].x/32 && yy == bad_guys[i].y/32)
        bad_guys[i].alive = NO;


  if (yy >= 0 && yy < 15 && xx >= 0 && xx <= current_level.width)
    current_level.tiles[yy][xx] = c;
}

/* Save data for this level: */
void savelevel(void)
{
  FILE * fi;
  char * filename;
  int y;
  char str[80];

  /* Save data file: */

  filename = (char *) malloc(sizeof(char) * (strlen(DATA_PREFIX) + 20) + strlen(level_subset));
  sprintf(filename, "%s/levels/%s/level%d.dat", DATA_PREFIX, level_subset, level);
  fi = fopen(filename, "w");
  if (fi == NULL)
    {
      perror(filename);
      st_shutdown();
      free(filename);
      exit(-1);
    }
  free(filename);


  /* sptrinf("# Level created by SuperTux built-in editor", fi); */

  fputs(current_level.name, fi);
  fputs("\n", fi);
  fputs(current_level.theme, fi);
  fputs("\n", fi);
  sprintf(str, "%d\n", current_level.time_left);	/* time */
  fputs(str, fi);
  fputs(current_level.song_title, fi);	/* song filename */
  sprintf(str, "\n%d\n", current_level.bkgd_red);	/* red background color */
  fputs(str, fi);
  sprintf(str, "%d\n", current_level.bkgd_green);	/* green background color */
  fputs(str, fi);
  sprintf(str, "%d\n", current_level.bkgd_blue);	/* blue background color */
  fputs(str, fi);
  sprintf(str, "%d\n", current_level.width);	/* level width */
  fputs(str, fi);

  for(y = 0; y < 15; ++y)
    {
      fputs(current_level.tiles[y], fi);
      fputs("\n", fi);
    }

  fclose(fi);

  drawcenteredtext("SAVED!", 240, letters_gold, NO_UPDATE, 1);
  SDL_Flip(screen);
  SDL_Delay(1000);
}

void showhelp()
{
  SDL_Event event;
  int done;
  char *text[] = {
                   "X/x - Brick0",
                   "Y/y - Brick1",
                   "A/B/! - Box full",
                   "a - Box empty",
                   "C-F - Cloud0",
                   "c-f - Cloud1",
                   "G-J - Bkgd0",
                   "g-j - Bkgd1",
                   "# - Solid0",
                   "[ - Solid1",
                   "= - Solid2",
                   "] - Solid3",
                   "$ - Distro",
                   "^ - Waves",
                   "* - Poletop",
                   "| - Pole",
                   "\\ - Flag",
                   "& - Water",
                   "0-2 - BadGuys",
                   "./Del - Remove tile",
                   "Esc - Menu"};
  int i;


  drawcenteredtext("- Help -", 30, letters_red, NO_UPDATE, 1);
  drawtext("Keys:", 80, 60, letters_gold, NO_UPDATE, 1);

  for(i = 0; i < sizeof(text)/sizeof(char *); i++)
    drawtext(text[i], 40, 90+(i*16), letters_blue, NO_UPDATE, 1);

  SDL_Flip(screen);

  done = 0;

  while(done == 0)
    while(SDL_PollEvent(&event))
      switch(event.type)
        {
        case SDL_KEYDOWN:		// key pressed
          done = 1;
          break;
        default:
          break;
        }
}
