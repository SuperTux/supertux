/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/*  December 28, 2003 - March 15, 2004 */

/* leveleditor.c - A built-in level editor for SuperTux
 Ricardo Cruz <rick2@aeiou.pt>
 Tobias Glaesser <tobi.web@gmx.de>                      */

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
void le_new_subset(char *subset_name);

void le_highlight_selection();

void apply_level_settings_menu();
void update_subset_settings_menu();
void save_subset_settings_menu();
void le_update_buttons(char*);

/* leveleditor internals */
static string_list_type level_subsets;
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
static int le_mouse_pressed[2];
static button_type le_save_level_bt;
static button_type le_test_level_bt;
static button_type le_next_level_bt;
static button_type le_previous_level_bt;
static button_type le_move_right_bt;
static button_type le_move_left_bt;
static button_type le_rubber_bt;
static button_type le_select_mode_one_bt;
static button_type le_select_mode_two_bt;
static button_type le_settings_bt;
static button_type le_bad_bt;
static button_type le_bkgd_bt;
static button_type le_fgd_bt;
static button_panel_type le_bkgd_panel;
static button_panel_type le_fgd_panel;
static button_panel_type le_bad_panel;
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
                case 3:
                  update_subset_settings_menu();
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
                case 13:
                  apply_level_settings_menu();
                  menu_set_current(&leveleditor_menu);
                  break;
                default:
                  show_menu = YES;
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
                      subset_load(&le_level_subset,level_subsets.item[i-2]);
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
		      le_update_buttons(le_current_level->theme);
                      le_set_defaults();
                      level_load_gfx(le_current_level);
                      le_activate_bad_guys();
                      show_menu = YES;
                    }
                  break;
                }
            }
          else if(current_menu == &subset_new_menu)
            {
              if(subset_new_menu.item[2].input[0] == '\0')
                subset_new_menu.item[3].kind = MN_DEACTIVE;
              else
                {
                  subset_new_menu.item[3].kind = MN_ACTION;

                  switch (i = menu_check(&subset_new_menu))
                    {
                    case 3:
                      le_new_subset(subset_new_menu.item[2].input);
                      subset_load(&le_level_subset,subset_new_menu.item[2].input);
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
		      le_update_buttons(le_current_level->theme);
                      le_set_defaults();
                      level_load_gfx(le_current_level);
                      le_activate_bad_guys();
                      menu_item_change_input(&subset_new_menu.item[2],"");
                      show_menu = YES;
                      break;
                    }
                }
            }
          else if(current_menu == &subset_settings_menu)
            {
              if(strcmp(le_level_subset.title,subset_settings_menu.item[2].input) == 0 && strcmp(le_level_subset.description,subset_settings_menu.item[3].input) == 0  )
                subset_settings_menu.item[5].kind = MN_DEACTIVE;
              else
                subset_settings_menu.item[5].kind = MN_ACTION;

              switch (i = menu_check(&subset_settings_menu))
                {
                case 5:
                  save_subset_settings_menu();
                  show_menu = YES;
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

void le_default_level(st_level* plevel)
{
  int i,y;
  strcpy(plevel->name,"UnNamed");
  strcpy(plevel->theme,"antarctica");
  strcpy(plevel->song_title,"Mortimers_chipdisko.mod");
  strcpy(plevel->bkgd_image,"arctis.png");
  plevel->width = 21;
  plevel->time_left = 100;
  plevel->gravity = 10.;
  plevel->bkgd_red = 0;
  plevel->bkgd_green = 0;
  plevel->bkgd_blue = 0;

  for(i = 0; i < 15; ++i)
    {
      plevel->tiles[i] = (unsigned char*) malloc((plevel->width+1)*sizeof(unsigned char));
      plevel->tiles[i][plevel->width] = (unsigned char) '\0';
      for(y = 0; y < plevel->width; ++y)
        plevel->tiles[i][y] = (unsigned char) '.';
      plevel->tiles[i][plevel->width] = (unsigned char) '\0';
    }
}

void le_new_subset(char *subset_name)
{
  st_level new_lev;
  st_subset new_subset;
  new_subset.name = (char*) malloc((strlen(subset_name)+1)*sizeof(char));
  strcpy(new_subset.name,subset_name);
  new_subset.title = (char*) malloc((strlen("Unknown Title")+1)*sizeof(char));
  strcpy(new_subset.title,"Unknown Title");
  new_subset.description = (char*) malloc((strlen("No description so far.")+1)*sizeof(char));
  strcpy(new_subset.description,"No description so far.");
  subset_save(&new_subset);
  le_default_level(&new_lev);
  level_save(&new_lev,subset_name,1);
}

void le_update_buttons(char *theme)
{
  int i;
  char filename[1024];
  char pathname[1024];
  SDLKey key;
  string_list_type bkgd_files;
  string_list_type fgd_files;

  sprintf(pathname,"images/themes/%s",theme);
  bkgd_files =  dfiles(pathname,"bkgd-", NULL);
  string_list_sort(&bkgd_files);

  le_bkgd_panel.hidden = YES;
  key = SDLK_a;
  for(i = 0; i < bkgd_files.num_items; ++i)
    {
      sprintf(filename,"%s/%s",pathname,bkgd_files.item[i]);
      button_change_icon(&le_bkgd_panel.item[i],filename);
    }

  sprintf(pathname,"images/themes/%s",theme);
  fgd_files =  dfiles(pathname,"solid", NULL);
  string_list_sort(&fgd_files);
  key = SDLK_a;
  for(i = 0; i < fgd_files.num_items; ++i)
    {
      sprintf(filename,"%s/%s",pathname,fgd_files.item[i]);
      button_change_icon(&le_fgd_panel.item[i],filename);
    }

  string_list_free(&fgd_files);
  fgd_files =  dfiles(pathname,"brick", NULL);
  string_list_sort(&fgd_files);

  for(i = 0; i < fgd_files.num_items; ++i)
    {
      sprintf(filename,"%s/%s",pathname,fgd_files.item[i]);
      button_change_icon(&le_fgd_panel.item[i+14],filename);
    }
}

int le_init()
{
  int i,j;
  char str[80];
  char filename[1024];
  SDLKey key;
  string_list_type fgd_files;
  string_list_type bkgd_files;
  string_list_type bad_files;
  level_subsets = dsubdirs("/levels", "info");

  le_show_grid = YES;

  /*  level_changed = NO;*/
  fire = DOWN;
  done = 0;
  le_frame = 0;	/* support for frames in some tiles, like waves and bad guys */
  le_level_changed = NO;

  subset_init(&le_level_subset);

  le_current_level = NULL;

  le_current_tile = '.';
  le_mouse_pressed[LEFT] = NO;
  le_mouse_pressed[RIGHT] = NO;

  texture_load(&le_selection,DATA_PREFIX "/images/leveleditor/select.png", USE_ALPHA);

  /* Load buttons */
  button_load(&le_save_level_bt,"/images/icons/save.png","Save level", SDLK_F6,screen->w-64,32);
  button_load(&le_next_level_bt,"/images/icons/up.png","Next level", SDLK_PAGEUP,screen->w-64,0);
  button_load(&le_previous_level_bt,"/images/icons/down.png","Previous level",SDLK_PAGEDOWN,screen->w-32,0);
  button_load(&le_rubber_bt,"/images/icons/rubber.png","Rubber",SDLK_DELETE,screen->w-32,48);
  button_load(&le_select_mode_one_bt,"/images/icons/select-mode1.png","Select single tile",SDLK_F3,screen->w-64,48);
  button_load(&le_select_mode_two_bt,"/images/icons/select-mode2.png","Select multiple tiles",SDLK_F3,screen->w-64,64);
  button_load(&le_test_level_bt,"/images/icons/test-level.png","Test level",SDLK_F4,screen->w-64,screen->h - 64);
  button_load(&le_settings_bt,"/images/icons/settings.png","Level settings",SDLK_F5,screen->w-32,screen->h - 64);
  button_load(&le_move_left_bt,"/images/icons/left.png","Move left",SDLK_LEFT,0,0);
  button_load(&le_move_right_bt,"/images/icons/right.png","Move right",SDLK_RIGHT,screen->w-80,0);
  button_load(&le_fgd_bt,"/images/icons/fgd.png","Foreground tiles", SDLK_F7,screen->w-64,82);
  button_load(&le_bkgd_bt,"/images/icons/bgd.png","Background tiles", SDLK_F8,screen->w-43,82);
  button_load(&le_bad_bt,"/images/icons/emy.png","Enemies", SDLK_F9,screen->w-22,82);

  bkgd_files =  dfiles("images/themes/antarctica","bkgd-", NULL);
  string_list_sort(&bkgd_files);

  button_panel_init(&le_bkgd_panel, screen->w - 64,98, 64, 318);
  le_bkgd_panel.hidden = YES;
  key = SDLK_a;
  for(i = 0; i < bkgd_files.num_items; ++i)
    {
      sprintf(filename,"images/themes/antarctica/%s",bkgd_files.item[i]);
      button_panel_additem(&le_bkgd_panel,button_create(filename, "Background Tile",(SDLKey)((int)key+i),0,0),i);
    }

  string_list_free(&bkgd_files);
  bkgd_files = dfiles("images/shared","cloud-", NULL);
  string_list_sort(&bkgd_files);

  for(i = 0; i < bkgd_files.num_items; ++i)
    {
      sprintf(filename,"images/shared/%s",bkgd_files.item[i]);
      button_panel_additem(&le_bkgd_panel,button_create(filename, "Background Tile",(SDLKey)((int)key+i+8),0,0),i+8);
    }

  fgd_files =  dfiles("images/themes/antarctica","solid", NULL);
  string_list_sort(&fgd_files);
  key = SDLK_a;
  button_panel_init(&le_fgd_panel, screen->w - 64,98, 64, 318);
  for(i = 0; i < fgd_files.num_items; ++i)
    {
      sprintf(filename,"images/themes/antarctica/%s",fgd_files.item[i]);
      button_panel_additem(&le_fgd_panel,button_create(filename, "Foreground Tile",(SDLKey)((int)key+i),0,0),i);
    }

  string_list_free(&fgd_files);
  string_list_add_item(&fgd_files,"waves-0.png");
  string_list_add_item(&fgd_files,"water.png");
  string_list_add_item(&fgd_files,"pole.png");
  string_list_add_item(&fgd_files,"poletop.png");
  string_list_add_item(&fgd_files,"flag-0.png");
  string_list_add_item(&fgd_files,"box-empty.png");
  string_list_add_item(&fgd_files,"mints.png");
  string_list_add_item(&fgd_files,"distro-0.png");
  string_list_add_item(&fgd_files,"golden-herring.png");
  string_list_add_item(&fgd_files,"distro-0.png");

  for(i = 0; i < fgd_files.num_items; ++i)
    {
      sprintf(filename,"images/shared/%s",fgd_files.item[i]);
      button_panel_additem(&le_fgd_panel,button_create(filename, "Foreground Tile",(SDLKey)((int)key+i+4),0,0),i+4);
    }

  string_list_free(&fgd_files);
  fgd_files =  dfiles("images/themes/antarctica","brick", NULL);
  string_list_sort(&fgd_files);

  for(i = 0; i < fgd_files.num_items; ++i)
    {
      sprintf(filename,"images/themes/antarctica/%s",fgd_files.item[i]);
      button_panel_additem(&le_fgd_panel,button_create(filename, "Foreground Tile",(SDLKey)((int)key+i+14),0,0),i+14);
    }

  string_list_free(&fgd_files);
  string_list_add_item(&fgd_files,"distro-0.png");
  string_list_add_item(&fgd_files,"distro-0.png");
  for(i = 0; i < fgd_files.num_items; ++i)
    {
      sprintf(filename,"images/shared/%s",fgd_files.item[i]);
      button_panel_additem(&le_fgd_panel,button_create(filename, "Foreground Tile",(SDLKey)((int)key+i+16),0,0),i+16);
    }

  le_fgd_panel.item[10].bkgd = &le_fgd_panel.item[9].icon;
  le_fgd_panel.item[11].bkgd = &le_fgd_panel.item[9].icon;
  le_fgd_panel.item[12].bkgd = &le_fgd_panel.item[9].icon;
  le_fgd_panel.item[16].bkgd = &le_fgd_panel.item[14].icon;
  le_fgd_panel.item[17].bkgd = &le_fgd_panel.item[15].icon;

  bad_files;
  string_list_init(&bad_files);
  string_list_add_item(&bad_files,"bsod-left-0.png");
  string_list_add_item(&bad_files,"laptop-left-0.png");
  string_list_add_item(&bad_files,"bag-left-0.png");
  button_panel_init(&le_bad_panel, screen->w - 64,98, 64, 318);
  le_bad_panel.hidden = YES;
  key = SDLK_a;
  for(i = 0; i < bad_files.num_items; ++i)
    {
      sprintf(filename,"images/shared/%s",bad_files.item[i]);
      button_panel_additem(&le_bad_panel,button_create(filename, "Bad Guy",(SDLKey)((int)key+i),0,0),i);
    }

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
  for(i = 0; i < level_subsets.num_items; ++i)
    {
      menu_additem(&subset_load_menu,menu_item_create(MN_ACTION,level_subsets.item[i],0,0));
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
  menu_additem(&subset_settings_menu,menu_item_create(MN_TEXTFIELD,"Title",0,0));
  menu_additem(&subset_settings_menu,menu_item_create(MN_TEXTFIELD,"Description",0,0));
  menu_additem(&subset_settings_menu,menu_item_create(MN_HL,"",0,0));
  menu_additem(&subset_settings_menu,menu_item_create(MN_ACTION,"Save Changes",0,0));
  menu_additem(&subset_settings_menu,menu_item_create(MN_HL,"",0,0));
  menu_additem(&subset_settings_menu,menu_item_create(MN_BACK,"Back",0,0));

  menu_init(&level_settings_menu);
  level_settings_menu.arrange_left = YES;
  menu_additem(&level_settings_menu,menu_item_create(MN_LABEL,"Level Settings",0,0));
  menu_additem(&level_settings_menu,menu_item_create(MN_HL,"",0,0));
  menu_additem(&level_settings_menu,menu_item_create(MN_TEXTFIELD,"Name    ",0,0));
  menu_additem(&level_settings_menu,menu_item_create(MN_STRINGSELECT,"Theme   ",0,0));
  menu_additem(&level_settings_menu,menu_item_create(MN_STRINGSELECT,"Song    ",0,0));
  menu_additem(&level_settings_menu,menu_item_create(MN_STRINGSELECT,"Bg-Image",0,0));
  menu_additem(&level_settings_menu,menu_item_create(MN_NUMFIELD,"Length ",0,0));
  menu_additem(&level_settings_menu,menu_item_create(MN_NUMFIELD,"Time   ",0,0));
  menu_additem(&level_settings_menu,menu_item_create(MN_NUMFIELD,"Gravity",0,0));
  menu_additem(&level_settings_menu,menu_item_create(MN_NUMFIELD,"Red    ",0,0));
  menu_additem(&level_settings_menu,menu_item_create(MN_NUMFIELD,"Green  ",0,0));
  menu_additem(&level_settings_menu,menu_item_create(MN_NUMFIELD,"Blue   ",0,0));
  menu_additem(&level_settings_menu,menu_item_create(MN_HL,"",0,0));
  menu_additem(&level_settings_menu,menu_item_create(MN_ACTION,"Apply Changes",0,0));

  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

  return 0;
}

void update_level_settings_menu()
{
  char str[80];
  int i;

  menu_item_change_input(&level_settings_menu.item[2], le_current_level->name);
  sprintf(str,"%d",le_current_level->width);

  string_list_copy(level_settings_menu.item[3].list, dsubdirs("images/themes", "solid0.png"));
  string_list_copy(level_settings_menu.item[4].list, dfiles("music/",NULL, "-fast"));
  string_list_copy(level_settings_menu.item[5].list, dfiles("images/background",NULL, NULL));
  string_list_add_item(level_settings_menu.item[5].list,"");
  if((i = string_list_find(level_settings_menu.item[3].list,le_current_level->theme)) != -1)
    level_settings_menu.item[3].list->active_item = i;
  if((i = string_list_find(level_settings_menu.item[4].list,le_current_level->song_title)) != -1)
    level_settings_menu.item[4].list->active_item = i;
  if((i = string_list_find(level_settings_menu.item[5].list,le_current_level->bkgd_image)) != -1)
    level_settings_menu.item[5].list->active_item = i;

  menu_item_change_input(&level_settings_menu.item[6], str);
  sprintf(str,"%d",le_current_level->time_left);
  menu_item_change_input(&level_settings_menu.item[7], str);
  sprintf(str,"%2.0f",le_current_level->gravity);
  menu_item_change_input(&level_settings_menu.item[8], str);
  sprintf(str,"%d",le_current_level->bkgd_red);
  menu_item_change_input(&level_settings_menu.item[9], str);
  sprintf(str,"%d",le_current_level->bkgd_green);
  menu_item_change_input(&level_settings_menu.item[10], str);
  sprintf(str,"%d",le_current_level->bkgd_blue);
  menu_item_change_input(&level_settings_menu.item[11], str);
}

void update_subset_settings_menu()
{
  menu_item_change_input(&subset_settings_menu.item[2], le_level_subset.title);
  menu_item_change_input(&subset_settings_menu.item[3], le_level_subset.description);
}

void apply_level_settings_menu()
{
  int i,y,j;
  i = NO;

  strcpy(le_current_level->name,level_settings_menu.item[2].input);

  if(strcmp(le_current_level->bkgd_image,string_list_active(level_settings_menu.item[5].list)) != 0)
    {
      strcpy(le_current_level->bkgd_image,string_list_active(level_settings_menu.item[5].list));
      i = YES;
    }

  if(strcmp(le_current_level->theme,string_list_active(level_settings_menu.item[3].list)) != 0)
    {
      strcpy(le_current_level->theme,string_list_active(level_settings_menu.item[3].list));
      le_update_buttons(le_current_level->theme);
      i = YES;
    }

  if(i == YES)
    {
      level_free_gfx();
      level_load_gfx(le_current_level);
    }

  strcpy(le_current_level->song_title,string_list_active(level_settings_menu.item[4].list));

  i = le_current_level->width;
  le_current_level->width = atoi(level_settings_menu.item[6].input);
  if(le_current_level->width < i)
    {
      if(le_current_level->width < 21)
        le_current_level->width = 21;
      for(y = 0; y < 15; ++y)
        {
          le_current_level->tiles[y] = (unsigned char*) realloc(le_current_level->tiles[y],(le_current_level->width+1)*sizeof(unsigned char));
          le_current_level->tiles[y][le_current_level->width] = (unsigned char) '\0';
        }
    }
  else if(le_current_level->width > i)
    {
      for(y = 0; y < 15; ++y)
        {
          le_current_level->tiles[y] = (unsigned char*) realloc(le_current_level->tiles[y],(le_current_level->width+1)*sizeof(unsigned char));
          for(j = 0; j < le_current_level->width - i; ++j)
            le_current_level->tiles[y][i+j] = (unsigned char) '.';
          le_current_level->tiles[y][le_current_level->width] = (unsigned char) '\0';
        }
    }
  le_current_level->time_left = atoi(level_settings_menu.item[7].input);
  le_current_level->gravity = atof(level_settings_menu.item[8].input);
  le_current_level->bkgd_red = atoi(level_settings_menu.item[9].input);
  le_current_level->bkgd_green = atoi(level_settings_menu.item[10].input);
  le_current_level->bkgd_blue = atoi(level_settings_menu.item[11].input);
}

void save_subset_settings_menu()
{
  free(le_level_subset.title);
  le_level_subset.title = (char*) malloc(sizeof(char)*(strlen(subset_settings_menu.item[2].input)+1));
  strcpy(le_level_subset.title,subset_settings_menu.item[2].input);
  free(le_level_subset.description);
  le_level_subset.description = (char*) malloc(sizeof(char)*(strlen(subset_settings_menu.item[3].input)+1));
  strcpy(le_level_subset.description,subset_settings_menu.item[3].input);
  subset_save(&le_level_subset);
}

void le_goto_level(int levelnb)
{
  arrays_free();
  arrays_init();

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

  le_update_buttons(le_current_level->theme);

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

  texture_free(&le_selection);
  menu_free(&leveleditor_menu);
  menu_free(&subset_load_menu);
  menu_free(&subset_new_menu);
  menu_free(&subset_settings_menu);
  menu_free(&level_settings_menu);
  button_panel_free(&le_bkgd_panel);
  button_panel_free(&le_fgd_panel);
  button_panel_free(&le_bad_panel);
  button_free(&le_save_level_bt);
  button_free(&le_test_level_bt);
  button_free(&le_next_level_bt);
  button_free(&le_previous_level_bt);
  button_free(&le_move_right_bt);
  button_free(&le_move_left_bt);
  button_free(&le_rubber_bt);
  button_free(&le_select_mode_one_bt);
  button_free(&le_select_mode_two_bt);
  button_free(&le_settings_bt);
  button_free(&le_bad_bt);
  button_free(&le_bkgd_bt);
  button_free(&le_fgd_bt);

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
  switch(le_current_tile)
    {
    case 'B':
      texture_draw(&img_mints, 19 * 32, 14 * 32, NO_UPDATE);
      break;
    case '!':
      texture_draw(&img_golden_herring,19 * 32, 14 * 32, NO_UPDATE);
      break;
    case 'x':
    case 'y':
    case 'A':
      texture_draw(&img_distro[(le_frame / 5) % 4], 19 * 32, 14 * 32, NO_UPDATE);
      break;
    case '0':
      texture_draw(&img_bsod_left[(le_frame / 5) % 4],19 * 32, 14 * 32, NO_UPDATE);
      break;
    case '1':
      texture_draw(&img_laptop_left[(le_frame / 5) % 3],19 * 32, 14 * 32, NO_UPDATE);
      break;
    case '2':
      texture_draw(&img_money_left[0],19 * 32, 14 * 32, NO_UPDATE);
      break;
    default:
      break;
    }

  if(le_current_level != NULL)
    {
      button_draw(&le_save_level_bt);
      button_draw(&le_test_level_bt);
      button_draw(&le_next_level_bt);
      button_draw(&le_previous_level_bt);
      button_draw(&le_rubber_bt);
      button_draw(&le_select_mode_one_bt);
      button_draw(&le_select_mode_two_bt);
      button_draw(&le_settings_bt);
      button_draw(&le_move_right_bt);
      button_draw(&le_move_left_bt);
      button_draw(&le_bad_bt);
      button_draw(&le_bkgd_bt);
      button_draw(&le_fgd_bt);
      button_panel_draw(&le_bkgd_panel);
      button_panel_draw(&le_fgd_panel);
      button_panel_draw(&le_bad_panel);

      sprintf(str, "%d/%d", le_level,le_level_subset.levels);
      text_drawf(&white_text, str, -8, 16, A_RIGHT, A_NONE, 1, NO_UPDATE);

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
  button_type* pbutton;
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
                  if(key == SDLK_ESCAPE)
                    {
                      show_menu = NO;
                      menu_set_current(&leveleditor_menu);
                    }
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
                case SDLK_F9:
                  le_show_grid = !le_show_grid;
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
                  le_mouse_pressed[LEFT] = YES;

                  selection.x1 = event.motion.x + pos_x;
                  selection.y1 = event.motion.y;
                  selection.x2 = event.motion.x + pos_x;
                  selection.y2 = event.motion.y;
                }
              else if(event.button.button == SDL_BUTTON_RIGHT)
                le_mouse_pressed[RIGHT] = YES;
              break;
            case SDL_MOUSEBUTTONUP:
              if(event.button.button == SDL_BUTTON_LEFT)
                le_mouse_pressed[LEFT] = NO;
              else if(event.button.button == SDL_BUTTON_RIGHT)
                le_mouse_pressed[RIGHT] = NO;
              break;
            case SDL_MOUSEMOTION:
              if(!show_menu)
                {
                  x = event.motion.x;
                  y = event.motion.y;

                  cursor_x = ((int)(pos_x + x) / 32) * 32;
                  cursor_y = ((int) y / 32) * 32;

                  if(le_mouse_pressed[LEFT] == YES)
                    {
                      selection.x2 = x + pos_x;
                      selection.y2 = y;
                    }

                  if(le_mouse_pressed[RIGHT] == YES)
                    {
                      pos_x += -1 * event.motion.xrel;
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

      if(le_current_level != NULL)
        {
          if(event.type == SDL_KEYDOWN || event.type == SDL_KEYUP || ((event.type == SDL_MOUSEBUTTONDOWN || SDL_MOUSEMOTION) && (event.motion.x > screen->w-64 && event.motion.x < screen->w &&
              event.motion.y > 0 && event.motion.y < screen->h)))
            {
              le_mouse_pressed[LEFT] = NO;
              le_mouse_pressed[RIGHT] = NO;

              if(show_menu == NO)
                {
                  /* Check for button events */
                  button_event(&le_test_level_bt,&event);
                  if(button_get_state(&le_test_level_bt) == BN_CLICKED)
                    le_testlevel();
                  button_event(&le_save_level_bt,&event);
                  if(button_get_state(&le_save_level_bt) == BN_CLICKED)
                    level_save(le_current_level,le_level_subset.name,le_level);
                  button_event(&le_next_level_bt,&event);
                  if(button_get_state(&le_next_level_bt) == BN_CLICKED)
                    {
                      if(le_level < le_level_subset.levels)
                        {
                          le_goto_level(++le_level);
                        }
                      else
                        {
                          st_level new_lev;
                          char str[1024];
                          int d = 0;
                          sprintf(str,"Level %d doesn't exist.",le_level+1);
                          text_drawf(&white_text,str,0,-18,A_HMIDDLE,A_VMIDDLE,2,NO_UPDATE);
                          text_drawf(&white_text,"Do you want to create it?",0,0,A_HMIDDLE,A_VMIDDLE,2,NO_UPDATE);
                          text_drawf(&red_text,"(Y)es/(N)o",0,20,A_HMIDDLE,A_VMIDDLE,2,NO_UPDATE);
                          flipscreen();
                          while(d == 0)
                            {
                              while(SDL_PollEvent(&event))
                                switch(event.type)
                                  {
                                  case SDL_KEYDOWN:		// key pressed
                                    switch(event.key.keysym.sym)
                                      {
                                      case SDLK_y:
                                        le_default_level(&new_lev);
                                        level_save(&new_lev,le_level_subset.name,++le_level);
                                        le_level_subset.levels = le_level;
                                        le_goto_level(le_level);
                                        d = 1;
                                        break;
                                      case SDLK_n:
                                        d = 1;
                                        break;
                                      }
                                    break;
                                  default:
                                    break;
                                  }
                              SDL_Delay(50);
                            }
                        }
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

                  button_event(&le_bad_bt,&event);
                  if(button_get_state(&le_bad_bt) == BN_CLICKED)
                    {
                      le_bad_panel.hidden = NO;
                      le_fgd_panel.hidden = YES;
                      le_bkgd_panel.hidden = YES;
                    }

                  button_event(&le_fgd_bt,&event);
                  if(button_get_state(&le_fgd_bt) == BN_CLICKED)
                    {
                      le_bad_panel.hidden = YES;
                      le_fgd_panel.hidden = NO;
                      le_bkgd_panel.hidden = YES;
                    }
                  button_event(&le_bkgd_bt,&event);
                  if(button_get_state(&le_bkgd_bt) == BN_CLICKED)
                    {
                      le_bad_panel.hidden = YES;
                      le_fgd_panel.hidden = YES;
                      le_bkgd_panel.hidden = NO;
                    }
                  button_event(&le_settings_bt,&event);
                  if(button_get_state(&le_settings_bt) == BN_CLICKED)
                    {
                      if(show_menu == NO)
                        {
                          update_level_settings_menu();
                          menu_set_current(&level_settings_menu);
                          show_menu = YES;
                        }
                      else
                        {
                          menu_set_current(&leveleditor_menu);
                          show_menu = NO;
                        }
                    }
                  if((pbutton = button_panel_event(&le_bkgd_panel,&event)) != NULL)
                    {
                      if(button_get_state(pbutton) == BN_CLICKED)
                        {
                          char c = '\0';
                          if(pbutton->tag >= 0 && pbutton->tag <= 3)
                            c = 'G' + pbutton->tag;
                          else if(pbutton->tag >= 4 && pbutton->tag <= 7)
                            c = 'g' + pbutton->tag - 4;
                          else if(pbutton->tag >= 8 && pbutton->tag <= 11)
                            c = 'C' + pbutton->tag - 8;
                          else if(pbutton->tag >= 12 && pbutton->tag <= 15)
                            c = 'c' + pbutton->tag - 12;
                          if(c != '\0')
                            le_current_tile = c;
                        }
                    }
                  if((pbutton = button_panel_event(&le_fgd_panel,&event)) != NULL)
                    {
                      if(button_get_state(pbutton) == BN_CLICKED)
                        {
                          char c = '\0';
                          if(pbutton->tag == 0)
                            c = '#' ;
                          else if(pbutton->tag == 1)
                            c = '[';
                          else if(pbutton->tag == 2)
                            c = '=';
                          else if(pbutton->tag == 3)
                            c = ']';
                          else if(pbutton->tag == 4)
                            c = '^';
                          else if(pbutton->tag == 5)
                            c = '&';
                          else if(pbutton->tag == 6)
                            c = '|';
                          else if(pbutton->tag == 7)
                            c = '*';
                          else if(pbutton->tag == 8)
                            c = '\\';
                          else if(pbutton->tag == 9)
                            c = 'a';
                          else if(pbutton->tag == 10)
                            c = 'B';
                          else if(pbutton->tag == 11)
                            c = 'A';
                          else if(pbutton->tag == 12)
                            c = '!';
                          else if(pbutton->tag == 13)
                            c = '$';
                          else if(pbutton->tag == 14)
                            c = 'X';
                          else if(pbutton->tag == 15)
                            c = 'Y';
                          else if(pbutton->tag == 16)
                            c = 'x';
                          else if(pbutton->tag == 17)
                            c = 'y';
                          if(c != '\0')
                            le_current_tile = c;
                        }
                    }
                  if((pbutton = button_panel_event(&le_bad_panel,&event)) != NULL)
                    {
                      if(button_get_state(pbutton) == BN_CLICKED)
                        {
                          char c = '\0';
                          if(pbutton->tag >= 0 && pbutton->tag <= 2)
                            c = '0' + pbutton->tag;
                          if(c != '\0')
                            le_current_tile = c;
                        }
                    }
                }
              else
                {
                  button_event(&le_settings_bt,&event);
                  if(button_get_state(&le_settings_bt) == BN_CLICKED)
                    {
                      if(show_menu == NO)
                        {
                          update_level_settings_menu();
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
          if(show_menu == NO)
            {
              button_event(&le_move_left_bt,&event);
              button_event(&le_move_right_bt,&event);

              if(le_mouse_pressed[LEFT])
                {
                  le_change(cursor_x, cursor_y, le_current_tile);
                }
            }
        }
    }
  if(show_menu == NO)
    {
      if(button_get_state(&le_move_left_bt) == BN_PRESSED)
        {
          pos_x -= 192;
        }
      else if(button_get_state(&le_move_left_bt) == BN_HOVER)
        {
          pos_x -= 96;
        }

      if(button_get_state(&le_move_right_bt) == BN_PRESSED)
        {
          pos_x += 192;
        }
      else if(button_get_state(&le_move_right_bt) == BN_HOVER)
        {
          pos_x += 96;
        }
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
                   "  - This is SuperTux's built-in level editor -",
                   "It has been designed to be light and easy to use from the start.",
                   "",
                   "When you first load the level editor you are given a menu where you",
                   "can load level subsets, create a new level subset, edit the current",
                   "subset's settings, or simply quit the editor. You can access this menu",
                   "from the level editor at any time by pressing the escape key.",
                   "",
                   "To your right is your button bar. The center of this contains many",
                   "tiles you can use to make your level. To select a tile, click on it",
                   "with your left mouse button; your selection will be shown in the",
                   "bottom right corner of the button box. Click anywhere on your level",
                   "with the left mouse button to place that tile down. If you right click",
                   "a tile in the button bar, you can find out what its keyboard shortcut",
                   "is. The three buttons FGD, BGD and EMY let you pick from foreground,",
                   "background, and enemy tiles. The eraser lets you remove tiles.",
                   "The left and right arrow keys scroll back and forth through your level.",
                   "The button with the wrench and screwdriver, lets you change the",
                   "settings of your level, including how long it is or what music it will",
                   "play. When you are ready to give your level a test, click on the little",
                   "running Tux. If you like the changes you have made to your level,",
				   "press the red save key to keep them.",
                   "To change which level in your subset you are editing, press the white",
                   "up and down arrow keys at the top of the button box.",
				   "",
                   "Have fun making levels! If you make some good ones, send them to us on",
				   "the SuperTux mailing list!",
                   "- SuperTux team"
                 };


  text_drawf(&blue_text, "- Help -", 0, 30, A_HMIDDLE, A_TOP, 2, NO_UPDATE);

  for(i = 0; i < sizeof(text)/sizeof(char *); i++)
    text_draw(&white_small_text, text[i], 5, 80+(i*12), 1, NO_UPDATE);

  text_drawf(&gold_text, "Press Any Key to Continue", 0, 440, A_HMIDDLE, A_TOP, 1, NO_UPDATE);

  flipscreen();

  done = 0;

  while(done == 0)
    {
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
      SDL_Delay(50);
    }
}
