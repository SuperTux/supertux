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

#define MOUSE_LEFT_MARGIN 80
#define MOUSE_RIGHT_MARGIN (560-32)
/* right_margin should noticed that the cursor is 32 pixels,
   so it should subtract that value */
#define MOUSE_POS_SPEED 20

/* look */
#define SELECT_W 2 // size of the selections lines
#define SELECT_CLR 0, 255, 0, 255  // lines color (R, G, B, A)

/* gameloop funcs declerations */

void loadshared(void);
void unloadshared(void);

/* own declerations */
/* crutial ones (main loop) */
int le_init();
void le_quit();
void le_drawlevel();
void le_drawinterface();
void le_checkevents();
void le_change(float x, float y, unsigned char c);
void le_testlevel();
void le_showhelp();
void le_set_defaults(void);
void le_activate_bad_guys(void);

void le_highlight_selection();

/* leveleditor internals */
static char **level_subsets;
static int subsets_num;
static int le_level_changed;  /* if changes, ask for saving, when quiting*/
static int pos_x, cursor_x, cursor_y, cursor_tile, fire;
static int le_level;
static st_level* le_current_level;
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
static button_type le_select_mode_one_bt;
static button_type le_select_mode_two_bt;
static button_type le_bad_bsod_bt;
static button_type le_settings_bt;
static button_panel_type le_bt_panel;
static menu_type leveleditor_menu;
static menu_type subset_load_menu;
static menu_type subset_new_menu;
static menu_type subset_settings_menu;
static menu_type level_settings_menu;

static square selection;
static int le_selection_mode;
static SDL_Event event;

void le_activate_bad_guys(void)
{
  int x,y;

  /* Activate bad guys: */

  /* as oposed to the gameloop.c func, this one doesn't remove
  the badguys from tiles                                    */

  for (y = 0; y < 15; ++y)
    for (x = 0; x < le_current_level->width; ++x)
      if (le_current_level->tiles[y][x] >= '0' && le_current_level->tiles[y][x] <= '9')
        add_bad_guy(x * 32, y * 32, le_current_level->tiles[y][x] - '0');
}

void le_set_defaults()
{
  if(le_current_level != NULL)
    {
      /* Set defaults: */

      if(le_current_level->time_left == 0)
        le_current_level->time_left = 255;
    }
}

/* FIXME: Needs to be implemented. It should ask the user for the level(file)name and then let him create a new level based on this. */
void newlevel()
{}

/* FIXME: It should let select the user a level, which is in the leveldirectory and then load it. */
void selectlevel()
{}

int leveleditor(int levelnb)
{
  int last_time, now_time, i;

  le_level = levelnb;
  if(le_init() != 0)
    return 1;

  /* Clear screen: */

  clearscreen(0, 0, 0);
  updatescreen();

  while (SDL_PollEvent(&event))
  {}

  while(YES)
    {
      last_time = SDL_GetTicks();
      le_frame++;

      le_checkevents();

      if(le_current_level != NULL)
        {
          /* making events results to be in order */
          if(pos_x < 0)
            pos_x = 0;
          if(pos_x > (le_current_level->width * 32) - screen->w)
            pos_x = (le_current_level->width * 32) - screen->w;

          /* draw the level */
          le_drawlevel();
        }
      else
        clearscreen(0, 0, 0);

      /* draw editor interface */
      le_drawinterface();

      if(show_menu)
        {
          menu_process_current();
          if(current_menu == &leveleditor_menu)
            {
              switch (menu_check(&leveleditor_menu))
                {
                case 2:
                  show_menu = NO;
                  break;
                case 7:
                  done = DONE_LEVELEDITOR;
                  break;
                }
            }
          else if(current_menu == &level_settings_menu)
            {
              switch (menu_check(&level_settings_menu))
                {
                case 0:
                  break;
                case 1:
                  show_menu = YES;
                  break;
                case 4:
                  break;
                case 11:
                  strcpy(le_current_level->name,level_settings_menu.item[1].input);
                  break;
                }
            }
          else if(current_menu == &subset_load_menu)
            {
              switch (i = menu_check(&subset_load_menu))
                {
                case 0:
                  break;
                default:
                  if(i != -1)
                    {
                      subset_load(&le_level_subset,level_subsets[i-2]);
                      leveleditor_menu.item[3].kind = MN_GOTO;
                      le_level = 1;
                      arrays_init();
                      loadshared();
		      le_current_level = (st_level*) malloc(sizeof(st_level));
                      if(level_load(le_current_level, le_level_subset.name, le_level) != 0)
                        {
                          le_quit();
                          return 1;
                        }
                      le_set_defaults();
                      level_load_gfx(le_current_level);
                      le_activate_bad_guys();
                      show_menu = YES;
                    }
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
  int i;
  char str[80];
  level_subsets = NULL;
  level_subsets = dsubdirs("/levels", "info", &subsets_num);

  le_show_grid = YES;
  
  /*  level_changed = NO;*/
  fire = DOWN;
  done = 0;
  le_frame = 0;	/* support for frames in some tiles, like waves and bad guys */
  le_level_changed = NO;
  
  /*
    subset_load(&le_level_subset,"default");
    arrays_init();
    loadshared();
    le_set_defaults();
   
   
    if(level_load(&le_current_level, le_level_subset.name, le_level) != 0)
      {
        le_quit();
        return 1;
      }
    if(le_current_level.time_left == 0)
      le_current_level.time_left = 255;
   
    level_load_gfx(&le_current_level);
      le_activate_bad_guys();
    */
  le_current_level = NULL;

  le_current_tile = '.';
  le_mouse_pressed = NO;

  texture_load(&le_selection,DATA_PREFIX "/images/leveleditor/select.png", USE_ALPHA);

  /* Load buttons */
  button_load(&le_next_level_bt,"/images/icons/up.png","Test Level", SDLK_PAGEUP,screen->w-64,0);
  button_load(&le_previous_level_bt,"/images/icons/down.png","Test Level",SDLK_PAGEDOWN,screen->w-32,0);
  button_load(&le_rubber_bt,"/images/icons/rubber.png","Rubber",SDLK_DELETE,screen->w-64,32);
  button_load(&le_select_mode_one_bt,"/images/icons/select-mode1.png","Select Tile",SDLK_F3,screen->w-64,16);
  button_load(&le_select_mode_two_bt,"/images/icons/select-mode2.png","Select Tiles",SDLK_F3,screen->w-32,16);
  button_load(&le_test_level_bt,"/images/icons/test-level.png","Test Level",SDLK_F4,screen->w-64,screen->h - 64);
  button_load(&le_settings_bt,"/images/icons/settings.png","Level settings",SDLK_0,screen->w-32,screen->h - 64);
  button_panel_init(&le_bt_panel, screen->w - 64,64, 64, 380);

  menu_init(&leveleditor_menu);
  menu_additem(&leveleditor_menu,menu_item_create(MN_LABEL,"Level Editor Menu",0,0));
  menu_additem(&leveleditor_menu,menu_item_create(MN_HL,"",0,0));
  menu_additem(&leveleditor_menu,menu_item_create(MN_ACTION,"Return To Level Editor",0,0));
  menu_additem(&leveleditor_menu,menu_item_create(MN_DEACTIVE,"Level Subset Settings",0,&subset_settings_menu));
  menu_additem(&leveleditor_menu,menu_item_create(MN_GOTO,"Load Level Subset",0,&subset_load_menu));
  menu_additem(&leveleditor_menu,menu_item_create(MN_GOTO,"New Level Subset",0,&subset_new_menu));
  menu_additem(&leveleditor_menu,menu_item_create(MN_HL,"",0,0));
  menu_additem(&leveleditor_menu,menu_item_create(MN_ACTION,"Quit Level Editor",0,0));

  menu_reset();
  menu_set_current(&leveleditor_menu);
  show_menu = YES;
    
  menu_init(&subset_load_menu);
  menu_additem(&subset_load_menu,menu_item_create(MN_LABEL,"Load Level Subset",0,0));
  menu_additem(&subset_load_menu,menu_item_create(MN_HL,"",0,0));
  for(i = 0; i < subsets_num; ++i)
    {
      menu_additem(&subset_load_menu,menu_item_create(MN_ACTION,level_subsets[i],0,0));
    }
  menu_additem(&subset_load_menu,menu_item_create(MN_HL,"",0,0));
  menu_additem(&subset_load_menu,menu_item_create(MN_BACK,"Back",0,0));

  menu_init(&subset_new_menu);
  menu_additem(&subset_new_menu,menu_item_create(MN_LABEL,"New Level Subset",0,0));
  menu_additem(&subset_new_menu,menu_item_create(MN_HL,"",0,0));
  menu_additem(&subset_new_menu,menu_item_create(MN_TEXTFIELD,"Enter Name",0,0));
  menu_additem(&subset_new_menu,menu_item_create(MN_ACTION,"Create",0,0));
  menu_additem(&subset_new_menu,menu_item_create(MN_HL,"",0,0));
  menu_additem(&subset_new_menu,menu_item_create(MN_BACK,"Back",0,0));

  menu_init(&subset_settings_menu);
  menu_additem(&subset_settings_menu,menu_item_create(MN_LABEL,"Level Subset Settings",0,0));
  menu_additem(&subset_settings_menu,menu_item_create(MN_HL,"",0,0));
  menu_additem(&subset_settings_menu,menu_item_create(MN_BACK,"Back",0,0));

  menu_init(&level_settings_menu);
  level_settings_menu.arrange_left = YES;
  menu_additem(&level_settings_menu,menu_item_create(MN_LABEL,"Level Settings",0,0));
  menu_additem(&level_settings_menu,menu_item_create(MN_HL,"",0,0));
  menu_additem(&level_settings_menu,menu_item_create(MN_TEXTFIELD,"Name:",0,0));
  menu_additem(&level_settings_menu,menu_item_create(MN_TEXTFIELD,"Theme:",0,0));
  menu_additem(&level_settings_menu,menu_item_create(MN_TEXTFIELD,"Song:",0,0));
  menu_additem(&level_settings_menu,menu_item_create(MN_TEXTFIELD,"Background:",0,0));
  menu_additem(&level_settings_menu,menu_item_create(MN_NUMFIELD,"Length: ",0,0));
  menu_additem(&level_settings_menu,menu_item_create(MN_NUMFIELD,"Time:   ",0,0));
  menu_additem(&level_settings_menu,menu_item_create(MN_NUMFIELD,"Gravity:",0,0));
  menu_additem(&level_settings_menu,menu_item_create(MN_NUMFIELD,"Red:    ",0,0));
  menu_additem(&level_settings_menu,menu_item_create(MN_NUMFIELD,"Green:  ",0,0));
  menu_additem(&level_settings_menu,menu_item_create(MN_NUMFIELD,"Blue:   ",0,0));
  menu_additem(&level_settings_menu,menu_item_create(MN_HL,"",0,0));
  menu_additem(&level_settings_menu,menu_item_create(MN_ACTION,"Apply Changes",0,0));
  /*menu_additem(&level_settings_menu,menu_item_create(MN_GOTO,"Load Game",0,&load_game_menu));
  menu_additem(&level_settings_menu,menu_item_create(MN_GOTO,"Options",0,&options_menu));
  menu_additem(&level_settings_menu,menu_item_create(MN_ACTION,"Level editor",0,0));
  menu_additem(&level_settings_menu,menu_item_create(MN_ACTION,"Quit",0,0));*/

  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

  return 0;
}

void update_level_settings()
{
  char str[80];

  menu_item_change_input(&level_settings_menu.item[1], le_current_level->name);
  sprintf(str,"%d",le_current_level->width);
  menu_item_change_input(&level_settings_menu.item[5], str);
  sprintf(str,"%d",le_current_level->time_left);
  menu_item_change_input(&level_settings_menu.item[6], str);
  sprintf(str,"%2.0f",le_current_level->gravity);
  menu_item_change_input(&level_settings_menu.item[7], str);
  sprintf(str,"%d",le_current_level->bkgd_red);
  menu_item_change_input(&level_settings_menu.item[8], str);
  sprintf(str,"%d",le_current_level->bkgd_green);
  menu_item_change_input(&level_settings_menu.item[9], str);
  sprintf(str,"%d",le_current_level->bkgd_blue);
  menu_item_change_input(&level_settings_menu.item[10], str);
}

void le_goto_level(int levelnb)
{

  level_free(le_current_level);
  if(level_load(le_current_level, le_level_subset.name, levelnb) != 0)
    {
      level_load(le_current_level, le_level_subset.name, le_level);
    }
  else
    {
      le_level = levelnb;
    }

  le_set_defaults();

  level_free_gfx();
  level_load_gfx(le_current_level);

  le_activate_bad_guys();
}

void le_quit(void)
{
  /*if(level_changed == YES)
    if(askforsaving() == CANCEL)
      return;*/ //FIXME

  SDL_EnableKeyRepeat(0, 0);    // disables key repeating

  button_free(&le_test_level_bt);
  texture_free(&le_selection);
  menu_free(&leveleditor_menu);

  if(le_current_level != NULL)
    {
      level_free_gfx();
      level_free(le_current_level);
      unloadshared();
      arrays_free();
    }
}

void le_drawinterface()
{
  int x,y;
  char str[80];

  if(le_current_level != NULL)
    {
      /* draw a grid (if selected) */
      if(le_show_grid)
        {
          for(x = 0; x < 19; x++)
            fillrect(x*32 - ((int)pos_x % 32), 0, 1, screen->h, 225, 225, 225,255);
          for(y = 0; y < 15; y++)
            fillrect(0, y*32, screen->w - 32, 1, 225, 225, 225,255);
        }
    }

  if(le_selection_mode == CURSOR)
    texture_draw(&le_selection, cursor_x - pos_x, cursor_y, NO_UPDATE);
  else if(le_selection_mode == SQUARE)
    {
      int w, h;
      le_highlight_selection();
      /* draw current selection */
      w = selection.x2 - selection.x1;
      h = selection.y2 - selection.y1;
      fillrect(selection.x1 - pos_x, selection.y1, w, SELECT_W, SELECT_CLR);
      fillrect(selection.x1 - pos_x + w, selection.y1, SELECT_W, h, SELECT_CLR);
      fillrect(selection.x1 - pos_x, selection.y1 + h, w, SELECT_W, SELECT_CLR);
      fillrect(selection.x1 - pos_x, selection.y1, SELECT_W, h, SELECT_CLR);
    }


  /* draw button bar */
  fillrect(screen->w - 64, 0, 64, screen->h, 50, 50, 50,255);
  drawshape(19 * 32, 14 * 32, le_current_tile);

  if(le_current_level != NULL)
    {
      button_draw(&le_test_level_bt);
      button_draw(&le_next_level_bt);
      button_draw(&le_previous_level_bt);
      button_draw(&le_rubber_bt);
      button_draw(&le_select_mode_one_bt);
      button_draw(&le_select_mode_two_bt);
      button_draw(&le_bad_bsod_bt);
      button_draw(&le_settings_bt);
      button_panel_draw(&le_bt_panel);

      sprintf(str, "%d/%d", le_level,le_level_subset.levels);
      text_draw(&white_text, "LEV", 0, 0, 1, NO_UPDATE);
      text_draw(&gold_text, str, 80, 0, 1, NO_UPDATE);

      text_draw(&white_small_text, "F1 for Help", 10, 430, 1, NO_UPDATE);
    }
  else
    {
      if(show_menu == NO)
        text_draw(&white_small_text, "No Level Subset loaded - Press ESC and choose one in the menu", 10, 430, 1, NO_UPDATE);
      else
        text_draw(&white_small_text, "No Level Subset loaded", 10, 430, 1, NO_UPDATE);
    }

}

void le_drawlevel()
{
  int y,x,i,s;
  static char str[LEVEL_NAME_MAX];

  /* Draw the real background */
  if(le_current_level->bkgd_image[0] != '\0')
    {
      s = pos_x / 30;
      texture_draw_part(&img_bkgd,s,0,0,0,img_bkgd.w - s - 32, img_bkgd.h, NO_UPDATE);
      texture_draw_part(&img_bkgd,0,0,screen->w - s - 32 ,0,s,img_bkgd.h, NO_UPDATE);
    }
  else
    {
      clearscreen(le_current_level->bkgd_red, le_current_level->bkgd_green, le_current_level->bkgd_blue);
    }

  /*       clearscreen(current_level.bkgd_red, current_level.bkgd_green, current_level.bkgd_blue); */

  for (y = 0; y < 15; ++y)
    for (x = 0; x < 20; ++x)
      {
        drawshape(x * 32 - ((int)pos_x % 32), y * 32, le_current_level->tiles[y][x + (int)(pos_x / 32)]);

        /* draw whats inside stuff when cursor is selecting those */
        /* (draw them all the time - is this the right behaviour?) */
        switch(le_current_level->tiles[y][x + (int)(pos_x/32)])
          {
          case 'B':
            texture_draw(&img_mints, x * 32 - ((int)pos_x % 32), y*32, NO_UPDATE);
            break;
          case '!':
            texture_draw(&img_golden_herring, x * 32 - ((int)pos_x % 32), y*32, NO_UPDATE);
            break;
          case 'x':
          case 'y':
          case 'A':
            texture_draw(&img_distro[(frame / 5) % 4], x * 32 - ((int)pos_x % 32), y*32, NO_UPDATE);
            break;
          default:
            break;
          }
      }

  /* Draw the Bad guys: */
  for (i = 0; i < num_bad_guys; ++i)
    {
      if(bad_guys[i].base.alive == NO)
        continue;
      /* to support frames: img_bsod_left[(frame / 5) % 4] */
      if(bad_guys[i].kind == BAD_BSOD)
        texture_draw(&img_bsod_left[(le_frame / 5) % 4], bad_guys[i].base.x - pos_x, bad_guys[i].base.y, NO_UPDATE);
      else if(bad_guys[i].kind == BAD_LAPTOP)
        texture_draw(&img_laptop_left[(le_frame / 5) % 3], bad_guys[i].base.x - pos_x, bad_guys[i].base.y, NO_UPDATE);
      else if (bad_guys[i].kind == BAD_MONEY)
        texture_draw(&img_money_left[(le_frame / 5) % 2], bad_guys[i].base.x - pos_x, bad_guys[i].base.y, NO_UPDATE);
    }


  /* Draw the player: */
  /* for now, the position is fixed at (0, 240) */
  texture_draw(&tux_right[(frame / 5) % 3], 0 - pos_x, 240, NO_UPDATE);
}

void le_checkevents()
{
  SDLKey key;
  SDLMod keymod;
  int x,y;

  keymod = SDL_GetModState();

  while(SDL_PollEvent(&event))
    {
      /* testing SDL_KEYDOWN, SDL_KEYUP and SDL_QUIT events*/
      if(event.type == SDL_KEYDOWN || ((event.type == SDL_MOUSEBUTTONDOWN || SDL_MOUSEMOTION) && (event.motion.x > 0 && event.motion.x < screen->w - 64 &&
                                       event.motion.y > 0 && event.motion.y < screen->h)))
        {

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

                  if(cursor_x < pos_x + MOUSE_LEFT_MARGIN)
                    pos_x = cursor_x - MOUSE_LEFT_MARGIN;

                  break;
                case SDLK_RIGHT:
                  if(fire == DOWN)
                    cursor_x += KEY_CURSOR_SPEED;
                  else
                    cursor_x += KEY_CURSOR_FASTSPEED;

                  if(cursor_x > pos_x + MOUSE_RIGHT_MARGIN-32)
                    pos_x = cursor_x - MOUSE_RIGHT_MARGIN+32;

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
                case SDLK_HOME:
                  cursor_x = 0;
                  pos_x = cursor_x;
                  break;
                case SDLK_END:
                  cursor_x = (le_current_level->width * 32) - 32;
                  pos_x = cursor_x;
                  break;
                case SDLK_PAGEUP:
                  cursor_x -= PAGE_CURSOR_SPEED;

                  if(cursor_x < pos_x + MOUSE_LEFT_MARGIN)
                    pos_x = cursor_x - MOUSE_LEFT_MARGIN;

                  break;
                case SDLK_PAGEDOWN:
                  cursor_x += PAGE_CURSOR_SPEED;

                  if(cursor_x > pos_x + MOUSE_RIGHT_MARGIN-32)
                    pos_x = cursor_x - MOUSE_RIGHT_MARGIN+32;

                  break;
                case SDLK_F9:
                  le_show_grid = !le_show_grid;
                  break;
                case SDLK_PERIOD:
                  le_change(cursor_x, cursor_y, '.');
                  break;
                case SDLK_a:
                  if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
                    le_current_tile = 'A';
                  else
                    le_current_tile = 'a';
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
                  break;
                case SDLK_1:
                  if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
                    le_change(cursor_x, cursor_y, '!');
                  else		/* let's add a bad guy */
                    le_change(cursor_x, cursor_y, '1');
                  break;
                case SDLK_2:
                  le_change(cursor_x, cursor_y, '2');
                  break;
                case SDLK_PLUS:
                  if(keymod == KMOD_LSHIFT || keymod == KMOD_RSHIFT || keymod == KMOD_CAPS)
                    le_change(cursor_x, cursor_y, '*');
                  break;
                default:
                  break;
                }
              break;
            case SDL_KEYUP:	/* key released */
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

                  selection.x1 = event.motion.x + pos_x;
                  selection.y1 = event.motion.y;
                  selection.x2 = event.motion.x + pos_x;
                  selection.y2 = event.motion.y;
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

                  if(le_mouse_pressed == YES)
                    {
                      selection.x2 = x + pos_x;
                      selection.y2 = y;
                    }
                }
              break;
            case SDL_QUIT:	// window closed
              done = DONE_QUIT;
              break;
            default:
              break;
            }
        }


    }

  if(le_current_level != NULL)
    {
      if(event.type == SDL_KEYDOWN || ((event.type == SDL_MOUSEBUTTONDOWN || SDL_MOUSEMOTION) && (event.motion.x > screen->w-64 && event.motion.x < screen->w &&
                                       event.motion.y > 0 && event.motion.y < screen->h)))
        {
          /* Check for button events */
          button_event(&le_test_level_bt,&event);
          if(button_get_state(&le_test_level_bt) == BN_CLICKED)
            le_testlevel();
          button_event(&le_next_level_bt,&event);
          if(button_get_state(&le_next_level_bt) == BN_CLICKED)
            {
              if(le_level < le_level_subset.levels)
                le_goto_level(++le_level);
            }
          button_event(&le_previous_level_bt,&event);
          if(button_get_state(&le_previous_level_bt) == BN_CLICKED)
            {
              if(le_level > 1)
                le_goto_level(--le_level);
            }
          button_event(&le_rubber_bt,&event);
          if(button_get_state(&le_rubber_bt) == BN_CLICKED)
            le_current_tile = '.';
          button_event(&le_select_mode_one_bt,&event);
          if(button_get_state(&le_select_mode_one_bt) == BN_CLICKED)
            le_selection_mode = CURSOR;
          button_event(&le_select_mode_two_bt,&event);
          if(button_get_state(&le_select_mode_two_bt) == BN_CLICKED)
            le_selection_mode = SQUARE;
          button_event(&le_bad_bsod_bt,&event);
          if(button_get_state(&le_bad_bsod_bt) == BN_CLICKED)
            le_current_tile = '0';
          button_event(&le_settings_bt,&event);
          if(button_get_state(&le_settings_bt) == BN_CLICKED)
            {
              if(show_menu == NO)
                {
                  menu_set_current(&level_settings_menu);
                  show_menu = YES;
                }
              else
                {
                  menu_set_current(&leveleditor_menu);
                  show_menu = NO;
                }
            }
        }
    }

  if(le_mouse_pressed)
    {
      le_change(cursor_x, cursor_y, le_current_tile);
    }

}

void le_highlight_selection()
{
  int x,y,i;
  int x1, x2, y1, y2;

  if(selection.x1 < selection.x2)
    {
      x1 = selection.x1;
      x2 = selection.x2;
    }
  else
    {
      x1 = selection.x2;
      x2 = selection.x1;
    }
  if(selection.y1 < selection.y2)
    {
      y1 = selection.y1;
      y2 = selection.y2;
    }
  else
    {
      y1 = selection.y2;
      y2 = selection.y1;
    }

  x1 /= 32;
  x2 /= 32;
  y1 /= 32;
  y2 /= 32;

  fillrect(x1*32-pos_x, y1*32,32* (x2 - x1 + 1),32 * (y2 - y1 + 1),173,234,177,103);
}

void le_change(float x, float y, unsigned char c)
{
  if(le_current_level != NULL)
    {
      int xx,yy,i;
      int x1, x2, y1, y2;

      /*  level_changed = YES; */

      switch(le_selection_mode)
        {
        case CURSOR:
          level_change(le_current_level,x,y,c);

          yy = ((int)y / 32);
          xx = ((int)x / 32);

          /* if there is a bad guy over there, remove it */
          for(i = 0; i < num_bad_guys; ++i)
            if (bad_guys[i].base.alive)
              if(xx == bad_guys[i].base.x/32 && yy == bad_guys[i].base.y/32)
                bad_guys[i].base.alive = NO;

          if(c == '0')  /* if it's a bad guy */
            add_bad_guy(xx*32, yy*32, BAD_BSOD);
          else if(c == '1')
            add_bad_guy(xx*32, yy*32, BAD_LAPTOP);
          else if(c == '2')
            add_bad_guy(xx*32, yy*32, BAD_MONEY);

          break;
        case SQUARE:
          if(selection.x1 < selection.x2)
            {
              x1 = selection.x1;
              x2 = selection.x2;
            }
          else
            {
              x1 = selection.x2;
              x2 = selection.x1;
            }
          if(selection.y1 < selection.y2)
            {
              y1 = selection.y1;
              y2 = selection.y2;
            }
          else
            {
              y1 = selection.y2;
              y2 = selection.y1;
            }

          x1 /= 32;
          x2 /= 32;
          y1 /= 32;
          y2 /= 32;

          /* if there is a bad guy over there, remove it */
          for(i = 0; i < num_bad_guys; ++i)
            if(bad_guys[i].base.alive)
              if(bad_guys[i].base.x/32 >= x1 && bad_guys[i].base.x/32 <= x2
                  && bad_guys[i].base.y/32 >= y1 && bad_guys[i].base.y/32 <= y2)
                bad_guys[i].base.alive = NO;

          for(xx = x1; xx <= x2; xx++)
            for(yy = y1; yy <= y2; yy++)
              {
                level_change(le_current_level, xx*32, yy*32, c);

                if(c == '0')  // if it's a bad guy
                  add_bad_guy(xx*32, yy*32, BAD_BSOD);
                else if(c == '1')
                  add_bad_guy(xx*32, yy*32, BAD_LAPTOP);
                else if(c == '2')
                  add_bad_guy(xx*32, yy*32, BAD_MONEY);
              }
          break;
        default:
          break;
        }
    }
}

void le_testlevel()
{
  level_save(le_current_level,"test",le_level);
  gameloop("test",le_level, ST_GL_TEST);
  menu_set_current(&leveleditor_menu);
  arrays_init();
  level_load_gfx(le_current_level);
  loadshared();
  le_activate_bad_guys();
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
                   "F3 - Change Selection Mode",
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
