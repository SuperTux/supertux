
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/*  December 28, 2003 - February 1st, 2004 */

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

#include "screen.h"
#include "defines.h"
#include "globals.h"
#include "setup.h"
#include "menu.h"
#include "level.h"
#include "badguy.h"
#include "gameloop.h"
#include "scene.h"

/* definitions to aid development */
#define DONE_LEVELEDITOR 1
#define DONE_QUIT        2
#define DONE_CHANGELEVEL 3

/* definitions that affect gameplay */
#define KEY_CURSOR_SPEED 32
#define KEY_CURSOR_FASTSPEED 64
// when pagedown/up pressed speed:
#define PAGE_CURSOR_SPEED 13*32

#define CURSOR_LEFT_MARGIN 96
#define CURSOR_RIGHT_MARGIN 512
/* right_margin should noticed that the cursor is 32 pixels,
   so it should subtract that value */

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
 
  flipscreen();
 
  SDL_Delay(1000);
*/

/* gameloop funcs declerations */

void loadshared(void);
void unloadshared(void);

/* own declerations */

void savelevel();
void le_change(float x, float y, unsigned char c);
void showhelp();
void le_set_defaults(void);
void le_activate_bad_guys(void);

/* global variables (based on the gameloop ones) */

int level;
st_level current_level;
char level_subset[100];
int show_grid;

int frame;
texture_type selection;
int last_time, now_time;

void le_quit(void)
{
  unloadlevelgfx();
  unloadshared();
  arrays_free();
  texture_free(&selection);
}

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
{}

/* FIXME: It should let select the user a level, which is in the leveldirectory and then load it. */
void selectlevel()
{}

int leveleditor()
{
  char str[LEVEL_NAME_MAX];
  int done;
  int x, y, i;	/* for cicles */
  int pos_x, cursor_x, cursor_y, cursor_tile, fire;
  SDL_Event event;
  SDLKey key;
  SDLMod keymod;

  strcpy(level_subset,"default");
  show_grid = NO;
  
  level = 1;

  initmenu();
  menumenu = MENU_LEVELEDITOR;
  show_menu = YES;

  frame = 0;	/* support for frames in some tiles, like waves and bad guys */

  arrays_init();
  
  loadshared();
  set_defaults();

  loadlevel(&current_level,"default",level);
  loadlevelgfx(&current_level);

  le_activate_bad_guys();
  le_set_defaults();

  texture_load(&selection,DATA_PREFIX "/images/leveleditor/select.png", USE_ALPHA);

  done = 0;
  pos_x = 0;
  cursor_x = 3*32;
  cursor_y = 2*32;
  fire = DOWN;

  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

  while(1)
    {
      clearscreen(current_level.bkgd_red, current_level.bkgd_green, current_level.bkgd_blue);

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
                  menu_event(&event.key.keysym);
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

                  if(cursor_x > (current_level.width*32) - 32)
                    cursor_x = (current_level.width*32) - 32;
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

                  if(cursor_y > screen->h-32)
                    cursor_y = screen->h-32;
                  break;
                case SDLK_LCTRL:
                  fire =UP;
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
                case SDLK_PAGEUP:
                  cursor_x -= PAGE_CURSOR_SPEED;

									if(cursor_x < 0)
										cursor_x = 0;
                  break;
                case SDLK_PAGEDOWN:
                  cursor_x += PAGE_CURSOR_SPEED;

                  if(cursor_x > (current_level.width*32) - 32)
                    cursor_x = (current_level.width*32) - 32;
                  break;
                case SDLK_F9:
                  if(!show_grid)
                    show_grid = YES;
                  else
                    show_grid = NO;
                  break;
                case SDLK_PERIOD:
                  le_change(cursor_x, cursor_y, '.');
                  break;
                case SDLK_a:
                  if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
                    le_change(cursor_x, cursor_y, 'A');
                  else
                    le_change(cursor_x, cursor_y, 'a');
                  break;
                case SDLK_b:
                  if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
                    le_change(cursor_x, cursor_y, 'B');
                  break;
                case SDLK_c:
                  if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
                    le_change(cursor_x, cursor_y, 'C');
                  else
                    le_change(cursor_x, cursor_y, 'c');
                  break;
                case SDLK_d:
                  if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
                    le_change(cursor_x, cursor_y, 'D');
                  else
                    le_change(cursor_x, cursor_y, 'd');
                  break;
                case SDLK_e:
                  if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
                    le_change(cursor_x, cursor_y, 'E');
                  else
                    le_change(cursor_x, cursor_y, 'e');
                  break;
                case SDLK_f:
                  if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
                    le_change(cursor_x, cursor_y, 'F');
                  else
                    le_change(cursor_x, cursor_y, 'f');
                  break;
                case SDLK_g:
                  if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
                    le_change(cursor_x, cursor_y, 'G');
                  else
                    le_change(cursor_x, cursor_y, 'g');
                  break;
                case SDLK_h:
                  if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
                    le_change(cursor_x, cursor_y, 'H');
                  else
                    le_change(cursor_x, cursor_y, 'h');
                  break;
                case SDLK_i:
                  if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
                    le_change(cursor_x, cursor_y, 'I');
                  else
                    le_change(cursor_x, cursor_y, 'i');
                  break;
                case SDLK_j:
                  if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
                    le_change(cursor_x, cursor_y, 'J');
                  else
                    le_change(cursor_x, cursor_y, 'j');
                  break;
                case SDLK_x:
                  if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
                    le_change(cursor_x, cursor_y, 'X');
                  else
                    le_change(cursor_x, cursor_y, 'x');
                  break;
                case SDLK_y:
                  if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
                    le_change(cursor_x, cursor_y, 'Y');
                  else
                    le_change(cursor_x, cursor_y, 'y');
                  break;
                case SDLK_LEFTBRACKET:
                  le_change(cursor_x, cursor_y, '[');
                  break;
                case SDLK_RIGHTBRACKET:
                  le_change(cursor_x, cursor_y, ']');
                  break;
                case SDLK_HASH:
                case SDLK_3:
                  if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
                    le_change(cursor_x, cursor_y, '#');
                  break;
                case SDLK_DOLLAR:
                case SDLK_4:
                  if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
                    le_change(cursor_x, cursor_y, '$');
                  break;
                case SDLK_BACKSLASH:
                  if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
                    le_change(cursor_x, cursor_y, '|');
                  else
                    le_change(cursor_x, cursor_y, '\\');
                  break;
                case SDLK_CARET:
                  le_change(cursor_x, cursor_y, '^');
                  break;
                case SDLK_AMPERSAND:
                case SDLK_6:
                  if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
                    le_change(cursor_x, cursor_y, '&');
                  break;
                case SDLK_EQUALS:
                case SDLK_0:
                  if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
                    le_change(cursor_x, cursor_y, '=');
                  else		/* let's add a bad guy */
                    le_change(cursor_x, cursor_y, '0');
		 
		  add_bad_guy((((int)cursor_x/32)*32), (((int)cursor_y/32)*32), BAD_BSOD);
                  break;
                case SDLK_1:
                  if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
                    le_change(cursor_x, cursor_y, '!');
                  else		/* let's add a bad guy */
                    le_change(cursor_x, cursor_y, '1');

		  add_bad_guy((((int)cursor_x/32)*32), (((int)cursor_y/32)*32), BAD_LAPTOP);
                  break;
                case SDLK_2:
                  le_change(cursor_x, cursor_y, '2');

		  add_bad_guy((((int)cursor_x/32)*32), (((int)cursor_y/32)*32), BAD_MONEY);
                  break;
                case SDLK_PLUS:
                  if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
                    le_change(cursor_x, cursor_y, '*');
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
              /*            case SDL_MOUSEBUTTONDOWN:
                            if(event.button.button == SDL_BUTTON_LEFT)
                              {
              This will draw current tile in the cursor position, when the interface is done.
                              }
                            break;*/
            case SDL_MOUSEMOTION:
              if(!show_menu)
                {
                  x = event.motion.x;
                  y = event.motion.y;

                  cursor_x = ((int)(pos_x + x) / 32) * 32;
                  cursor_y = ((int) y / 32) * 32;
                }
              break;
            case SDL_QUIT:	// window closed
              done = DONE_QUIT;
              break;
            default:
              break;
            }
        }

      /* mouse movements */
      /*      x = event.motion.x;
            if(x < MOUSE_LEFT_MARGIN)
              pos_x -= MOUSE_POS_SPEED;
            else if(x > MOUSE_RIGHT_MARGIN)
              pos_x += MOUSE_POS_SPEED;*/


      if(cursor_x < pos_x + CURSOR_LEFT_MARGIN)
        pos_x = cursor_x - CURSOR_LEFT_MARGIN;

      if(cursor_x > pos_x + CURSOR_RIGHT_MARGIN)
        pos_x = cursor_x - CURSOR_RIGHT_MARGIN;

      if(pos_x < 0)
        pos_x = 0;
      if(pos_x > (current_level.width * 32) - screen->w)
        pos_x = (current_level.width * 32) - screen->w;

      for (y = 0; y < 15; ++y)
        for (x = 0; x < 21; ++x)
          drawshape(x * 32, y * 32, current_level.tiles[y][x + (pos_x / 32)]);

/* draw whats inside stuff when cursor is selecting those */
cursor_tile = current_level.tiles[cursor_y/32][cursor_x/32];
switch(cursor_tile)
	{
	case 'B':
		texture_draw(&img_mints, cursor_x - pos_x, cursor_y, NO_UPDATE);
		break;
	case '!':
		texture_draw(&img_golden_herring, cursor_x - pos_x, cursor_y, NO_UPDATE);
		break;
	case 'x':
	case 'y':
	case 'A':
		texture_draw(&img_distro[(frame / 5) % 4], cursor_x - pos_x, cursor_y, NO_UPDATE);
		break;
	default:
		break;
	}

      /* Draw the Bad guys: */
      for (i = 0; i < num_bad_guys; ++i)
        {
          if(bad_guys[i].base.alive == NO)
            continue;
          /* to support frames: img_bsod_left[(frame / 5) % 4] */
          if(bad_guys[i].kind == BAD_BSOD)
            texture_draw(&img_bsod_left[(frame / 5) % 4], ((int)(bad_guys[i].base.x - pos_x)/32)*32, bad_guys[i].base.y, NO_UPDATE);
          else if(bad_guys[i].kind == BAD_LAPTOP)
            texture_draw(&img_laptop_left[(frame / 5) % 3], ((int)(bad_guys[i].base.x - pos_x)/32)*32, bad_guys[i].base.y, NO_UPDATE);
          else if (bad_guys[i].kind == BAD_MONEY)
            texture_draw(&img_money_left[(frame / 5) % 2], ((int)(bad_guys[i].base.x - pos_x)/32)*32, bad_guys[i].base.y, NO_UPDATE);
        }

/* draw a grid (if selected) */
if(show_grid)
	{
	for(x = 0; x < 21; x++)
		fillrect(x*32, 0, 1, 480, 225, 225, 225);
	for(y = 0; y < 15; y++)
		fillrect(0, y*32, 640, 1, 225, 225, 225);
	}

      texture_draw(&selection, ((int)(cursor_x - pos_x)/32)*32, cursor_y, NO_UPDATE);

      sprintf(str, "%d", current_level.time_left);
      text_draw(&blue_text, "TIME", 324, 0, 1, NO_UPDATE);
      text_draw(&gold_text, str, 404, 0, 1, NO_UPDATE);

      sprintf(str, "%s", current_level.name);
      text_draw(&blue_text, "NAME", 0, 0, 1, NO_UPDATE);
      text_draw(&gold_text, str, 80, 0, 1, NO_UPDATE);

      text_draw(&blue_text, "F1 for Help", 10, 430, 1, NO_UPDATE);

      if(show_menu)
        {
          done = drawmenu();
          if(done)
	  {
	    le_quit();
            return 0;
	  }
        }
      if(done == DONE_QUIT)
        {
	le_quit();
        return 1;
	}

      SDL_Delay(50);
      now_time = SDL_GetTicks();
      if (now_time < last_time + FPS)
        SDL_Delay(last_time + FPS - now_time);	/* delay some time */

      flipscreen();
    }

  return done;
}

void le_change(float x, float y, unsigned char c)
{
int i;
int xx, yy;

  level_change(&current_level,x,y,c);

  yy = (y / 32);
  xx = (x / 32);

  /* if there is a bad guy over there, remove it */
  for(i = 0; i < num_bad_guys; ++i)
    if (bad_guys[i].base.alive)
      if(xx == bad_guys[i].base.x/32 && yy == bad_guys[i].base.y/32)
        bad_guys[i].base.alive = NO;
}

void showhelp()
{
  SDL_Event event;
  int i, done;
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
                   "F9 - Show/Hide Grid",
                   "Esc - Menu"};


  text_drawf(&red_text, "- Help -", 0, 30, A_HMIDDLE, A_TOP, 2, NO_UPDATE);
  text_draw(&gold_text, "Keys:", 80, 60, 1, NO_UPDATE);
  
  for(i = 0; i < sizeof(text)/sizeof(char *); i++)
    text_draw(&blue_text, text[i], 40, 90+(i*16), 1, NO_UPDATE);

  text_drawf(&gold_text, "Press Any Key to Continue", 0, 460, A_HMIDDLE, A_TOP, 1, NO_UPDATE);

  flipscreen();

  done = 0;

  while(done == 0)
    while(SDL_PollEvent(&event))
      switch(event.type)
        {
        case SDL_MOUSEBUTTONDOWN:		// mouse pressed
        case SDL_KEYDOWN:		// key pressed
          done = 1;
          break;
        default:
          break;
        }
}
