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
#include "scene.h"
#include "button.h"

/* definitions to aid development */
#define DONE_LEVELEDITOR 1
#define DONE_QUIT        2
#define DONE_CHANGELEVEL 3

/* definitions that affect gameplay */
#define KEY_CURSOR_SPEED 32
#define KEY_CURSOR_FASTSPEED 64
/* when pagedown/up pressed speed:*/
#define PAGE_CURSOR_SPEED 13*32

#define CURSOR_LEFT_MARGIN 96
#define CURSOR_RIGHT_MARGIN 600
/* right_margin should noticed that the cursor is 32 pixels,
   so it should subtract that value */

#define MOUSE_LEFT_MARGIN 32
#define MOUSE_RIGHT_MARGIN 608
#define MOUSE_POS_SPEED 32

/* gameloop funcs declerations */

void loadshared(void);
void unloadshared(void);

/* own declerations */
/* crutial ones (main loop) */
int le_init();
void le_quit();
void le_drawlevel();
void le_checkevents();
void le_change(float x, float y, unsigned char c);
void le_showhelp();
void le_set_defaults(void);
void le_activate_bad_guys(void);

/* leveleditor internals */
static int le_level_changed;  /* if changes, ask for saving, when quiting*/
static int pos_x, cursor_x, cursor_y, cursor_tile, fire;
static int le_level;
static st_level le_current_level;
static st_subset le_level_subset;
static int le_show_grid;
static int le_frame;
static texture_type le_selection;
static int done;
static char le_current_tile;
static int le_mouse_pressed;
static button_type le_test_level_bt;
static button_type le_next_level_bt;
static button_type le_previous_level_bt;
static button_type le_rubber_bt;

void le_activate_bad_guys(void)
{
  int x,y;

  /* Activate bad guys: */

  /* as oposed to the gameloop.c func, this one doesn't remove
  the badguys from tiles                                    */

  for (y = 0; y < 15; ++y)
    for (x = 0; x < le_current_level.width; ++x)
      if (le_current_level.tiles[y][x] >= '0' && le_current_level.tiles[y][x] <= '9')
        add_bad_guy(x * 32, y * 32, le_current_level.tiles[y][x] - '0');
}

void le_set_defaults()
{
  /* Set defaults: */

  if(le_current_level.time_left == 0)
    le_current_level.time_left = 255;
}

/* FIXME: Needs to be implemented. It should ask the user for the level(file)name and then let him create a new level based on this. */
void newlevel()
{}

/* FIXME: It should let select the user a level, which is in the leveldirectory and then load it. */
void selectlevel()
{}

int leveleditor(int levelnb)
{
  int last_time, now_time;

  le_level = levelnb;
  if(le_init() != 0)
    return 1;

  while(1)
    {
      last_time = SDL_GetTicks();
      le_frame++;

      le_checkevents();

      if(cursor_x < pos_x + CURSOR_LEFT_MARGIN)
        pos_x = cursor_x - CURSOR_LEFT_MARGIN;

      if(cursor_x > pos_x + CURSOR_RIGHT_MARGIN)
        pos_x = cursor_x - CURSOR_RIGHT_MARGIN;

      /* make sure we respect the borders */
      if(cursor_x < 0)
        cursor_x = 0;
      if(cursor_x > (le_current_level.width*32) - 32)
        cursor_x = (le_current_level.width*32) - 32;

      if(pos_x < 0)
        pos_x = 0;
      if(pos_x > (le_current_level.width * 32) - screen->w + 32)
        pos_x = (le_current_level.width * 32) - screen->w + 32;

      /* draw the level */
      le_drawlevel();

      if(show_menu)
        {
          menu_process_current();
          if(current_menu == &leveleditor_menu)
            {
              switch (menu_check(&leveleditor_menu))
                {
                case 0:
                  show_menu = NO;
                  break;
                case 4:
                  done = DONE_LEVELEDITOR;
                  break;
                }
            }
        }

      if(done)
        {
          le_quit();
          return 0;
        }

      if(done == DONE_QUIT)
        {
          le_quit();
          return 1;
        }

      SDL_Delay(25);
      now_time = SDL_GetTicks();
      if (now_time < last_time + FPS)
        SDL_Delay(last_time + FPS - now_time);	/* delay some time */

      flipscreen();
    }

  return done;
}

int le_init()
{
  subset_load(&le_level_subset,"default");
  le_show_grid = YES;

  done = 0;
  menu_reset();
  menu_set_current(&leveleditor_menu);
  le_frame = 0;	/* support for frames in some tiles, like waves and bad guys */

  arrays_init();
  loadshared();
  le_set_defaults();

  le_level_changed = NO;
  if(level_load(&le_current_level, le_level_subset.name, le_level) != 0)
    {
      le_quit();
      return 1;
    }
  if(le_current_level.time_left == 0)
    le_current_level.time_left = 255;

  level_load_gfx(&le_current_level);

  le_current_tile = '.';
  le_mouse_pressed = NO;
  le_activate_bad_guys();

  texture_load(&le_selection,DATA_PREFIX "/images/leveleditor/select.png", USE_ALPHA);
  button_load(&le_test_level_bt,"/images/icons/test-level.png","Test Level","Press this button to test the level that is currently being edited.",150,screen->h - 64);
  button_load(&le_next_level_bt,"/images/icons/up.png","Test Level","Press this button to test the level that is currently being edited.",screen->w-32,0);
  button_load(&le_previous_level_bt,"/images/icons/down.png","Test Level","Press this button to test the level that is currently being edited.",screen->w-32,16);
  button_load(&le_rubber_bt,"/images/icons/rubber.png","Test Level","Press this button to test the level that is currently being edited.",screen->w-32,32);

  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

  return 0;
}

void le_quit(void)
{
  /*if(level_changed == YES)
    if(askforsaving() == CANCEL)
      return;*/ //FIXME

  SDL_EnableKeyRepeat(0, 0);    // disables key repeating

  button_free(&le_test_level_bt);
  level_free_gfx();
  level_free(&le_current_level);
  unloadshared();
  arrays_free();
  texture_free(&le_selection);
}

void le_drawlevel()
{
  int y,x,i,s;
  static char str[LEVEL_NAME_MAX];

  /* Draw the real background */
  if(le_current_level.bkgd_image[0] != '\0')
    {
      s = pos_x / 30;
      texture_draw_part(&img_bkgd,s,0,0,0,img_bkgd.w - s - 32, img_bkgd.h, NO_UPDATE);
      texture_draw_part(&img_bkgd,0,0,screen->w - s - 32 ,0,s,img_bkgd.h, NO_UPDATE);
    }
  else
    {
      clearscreen(le_current_level.bkgd_red, le_current_level.bkgd_green, le_current_level.bkgd_blue);
    }

  /*       clearscreen(current_level.bkgd_red, current_level.bkgd_green, current_level.bkgd_blue); */

  for (y = 0; y < 15; ++y)
    for (x = 0; x < 19; ++x)
      {
        drawshape(x * 32, y * 32, le_current_level.tiles[y][x + (pos_x / 32)]);
      }

  /* draw whats inside stuff when cursor is selecting those */
  cursor_tile = le_current_level.tiles[cursor_y/32][cursor_x/32];
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
      texture_draw(&img_distro[(le_frame / 5) % 4], cursor_x - pos_x, cursor_y, NO_UPDATE);
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
        texture_draw(&img_bsod_left[(le_frame / 5) % 4], ((int)(bad_guys[i].base.x - pos_x)/32)*32, bad_guys[i].base.y, NO_UPDATE);
      else if(bad_guys[i].kind == BAD_LAPTOP)
        texture_draw(&img_laptop_left[(le_frame / 5) % 3], ((int)(bad_guys[i].base.x - pos_x)/32)*32, bad_guys[i].base.y, NO_UPDATE);
      else if (bad_guys[i].kind == BAD_MONEY)
        texture_draw(&img_money_left[(le_frame / 5) % 2], ((int)(bad_guys[i].base.x - pos_x)/32)*32, bad_guys[i].base.y, NO_UPDATE);
    }

  /* draw a grid (if selected) */
  if(le_show_grid)
    {
      for(x = 0; x < 19; x++)
        fillrect(x*32, 0, 1, screen->h, 225, 225, 225,255);
      for(y = 0; y < 15; y++)
        fillrect(0, y*32, screen->w - 32, 1, 225, 225, 225,255);
    }

  fillrect(screen->w - 32, 0, 32, screen->h, 50, 50, 50,255);
  drawshape(19 * 32, 14 * 32, le_current_tile);

  button_draw(&le_test_level_bt);
  button_draw(&le_next_level_bt);
  button_draw(&le_previous_level_bt);
  button_draw(&le_rubber_bt);
  
  texture_draw(&le_selection, ((int)(cursor_x - pos_x)/32)*32, cursor_y, NO_UPDATE);

  sprintf(str, "%d", le_current_level.time_left);
  text_draw(&white_text, "TIME", 324, 0, 1, NO_UPDATE);
  text_draw(&gold_text, str, 404, 0, 1, NO_UPDATE);

  text_draw(&white_text, "NAME", 0, 0, 1, NO_UPDATE);
  text_draw(&gold_text, le_current_level.name, 80, 0, 1, NO_UPDATE);

  sprintf(str, "%d/%d", le_level,le_level_subset.levels);
  text_draw(&white_text, "NUMB", 0, 20, 1, NO_UPDATE);
  text_draw(&gold_text, str, 80, 20, 1, NO_UPDATE);

  text_draw(&white_small_text, "F1 for Help", 10, 430, 1, NO_UPDATE);
  text_draw(&white_small_text, "F2 for Testing", 150, 430, 1, NO_UPDATE);
}

void le_checkevents()
{
  SDL_Event event;
  SDLKey key;
  SDLMod keymod;
  int x,y;

  keymod = SDL_GetModState();

  while(SDL_PollEvent(&event))
    {
      /* testing SDL_KEYDOWN, SDL_KEYUP and SDL_QUIT events*/
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
            case SDLK_ESCAPE:
              if(!show_menu)
                show_menu = YES;
              else
                show_menu = NO;
              break;
            case SDLK_LEFT:
              if(fire == DOWN)
                cursor_x -= KEY_CURSOR_SPEED;
              else
                cursor_x -= KEY_CURSOR_FASTSPEED;
              break;
            case SDLK_RIGHT:
              if(fire == DOWN)
                cursor_x += KEY_CURSOR_SPEED;
              else
                cursor_x += KEY_CURSOR_FASTSPEED;
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
              le_showhelp();
              break;
            case SDLK_F2:
              level_save(&le_current_level,"test",le_level);
              gameloop("test",le_level, ST_GL_TEST);
              menu_set_current(&leveleditor_menu);
              arrays_init();
              level_load_gfx(&le_current_level);
              loadshared();
              le_activate_bad_guys();
              break;
            case SDLK_HOME:
              cursor_x = 0;
              break;
            case SDLK_END:
              cursor_x = (le_current_level.width * 32) - 32;
              break;
            case SDLK_PAGEUP:
              cursor_x -= PAGE_CURSOR_SPEED;
              break;
            case SDLK_PAGEDOWN:
              cursor_x += PAGE_CURSOR_SPEED;
              break;
            case SDLK_F9:
              le_show_grid = !le_show_grid;
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
            default:
              break;
            }
          break;
        case SDL_MOUSEBUTTONDOWN:
          if(event.button.button == SDL_BUTTON_LEFT)
            {
              le_mouse_pressed = YES;
            }
          break;
        case SDL_MOUSEBUTTONUP:
          if(event.button.button == SDL_BUTTON_LEFT)
            {
              le_mouse_pressed = NO;
            }
          break;
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

  if(le_mouse_pressed)
    {
      le_change(cursor_x, cursor_y, le_current_tile);
      if(button_pressed(&le_test_level_bt,x,y))
        {
          level_save(&le_current_level,"test",le_level);
          gameloop("test",le_level, ST_GL_TEST);
          menu_set_current(&leveleditor_menu);
          arrays_init();
          level_load_gfx(&le_current_level);
          loadshared();
          le_activate_bad_guys();
        }
    }

}

void le_change(float x, float y, unsigned char c)
{
  int i;
  int xx, yy;

  level_change(&le_current_level,x,y,c);

  yy = ((int)y / 32);
  xx = ((int)x / 32);

  /* if there is a bad guy over there, remove it */
  for(i = 0; i < num_bad_guys; ++i)
    if (bad_guys[i].base.alive)
      if(xx == bad_guys[i].base.x/32 && yy == bad_guys[i].base.y/32)
        bad_guys[i].base.alive = NO;
}

void le_showhelp()
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


  text_drawf(&blue_text, "- Help -", 0, 30, A_HMIDDLE, A_TOP, 2, NO_UPDATE);
  text_draw(&gold_text, "Keys:", 80, 60, 1, NO_UPDATE);

  for(i = 0; i < sizeof(text)/sizeof(char *); i++)
    text_draw(&white_text, text[i], 40, 90+(i*16), 1, NO_UPDATE);

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
