/*
  title.c
  
  Super Tux - Title Screen
  
  by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/supertux/
  
  April 11, 2000 - March 15, 2004
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <SDL.h>
#include <SDL_image.h>

#ifdef LINUX
#include <pwd.h>
#include <sys/types.h>
#include <ctype.h>
#endif

#include "defines.h"
#include "globals.h"
#include "title.h"
#include "screen.h"
#include "high_scores.h"
#include "menu.h"
#include "texture.h"
#include "timer.h"
#include "setup.h"
#include "level.h"
#include "gameloop.h"
#include "leveleditor.h"

static texture_type bkg_title, img_choose_subset, anim1, anim2;
static SDL_Event event;
static SDLKey key;
static int quit, frame, pict, i;

void display_credits();

void draw_background()
{
  /* Draw the title background: */

  texture_draw_bg(&bkg_title, NO_UPDATE);

  /* Animate title screen: */

  pict = (frame / 5) % 3;

  if (pict == 0)
    texture_draw_part(&bkg_title, 560, 270, 560, 270, 80, 75, NO_UPDATE);
  else if (pict == 1)
    texture_draw(&anim1, 560, 270, NO_UPDATE);
  else if (pict == 2)
    texture_draw(&anim2, 560, 270, NO_UPDATE);
}

/* --- TITLE SCREEN --- */

int title(void)
{
  int done;
  char str[80];
  string_list_type level_subsets;
  st_subset subset;
  level_subsets = dsubdirs("/levels", "info");

  subset_init(&subset);

  /* Reset menu variables */
  menu_reset();
  menu_set_current(&main_menu);

  clearscreen(0, 0, 0);
  updatescreen();

  /* Load images: */

  texture_load(&bkg_title,DATA_PREFIX "/images/title/title.png", IGNORE_ALPHA);
  texture_load(&anim1,DATA_PREFIX "/images/title/title-anim2.png", IGNORE_ALPHA);
  texture_load(&anim2,DATA_PREFIX "/images/title/title-anim1.png", IGNORE_ALPHA);
  texture_load(&img_choose_subset,DATA_PREFIX "/images/status/choose-level-subset.png", USE_ALPHA);

  /* --- Main title loop: --- */

  done = 0;
  quit = 0;
  show_menu = 1;
  frame = 0;

  /* Draw the title background: */
  texture_draw_bg(&bkg_title, NO_UPDATE);

  load_hs();

  while (!done && !quit)
    {
      /* Handle events: */

      while (SDL_PollEvent(&event))
        {
          if (event.type == SDL_QUIT)
            {
              /* Quit event - quit: */
              quit = 1;
            }
          else if (event.type == SDL_KEYDOWN)
            {
              /* Keypress... */

              key = event.key.keysym.sym;

              /* Check for menu events */
              menu_event(&event.key.keysym);

              if (key == SDLK_ESCAPE)
                {
                  /* Escape: Quit: */

                  quit = 1;
                }
            }
#ifdef JOY_YES
          else if (event.type == SDL_JOYAXISMOTION && event.jaxis.axis == JOY_Y)
            {
              if (event.jaxis.value > 1024)
                menuaction = MN_DOWN;
              else if (event.jaxis.value < -1024)
                menuaction = MN_UP;
            }
          else if (event.type == SDL_JOYBUTTONDOWN)
            {
              /* Joystick button: Continue: */

              menuaction = MN_HIT;
            }
#endif

        }

      /* Draw the background: */
      draw_background();

      /* Draw the high score: */
      sprintf(str, "High score: %d", hs_score);
      text_drawf(&gold_text, str, 0, -40, A_HMIDDLE, A_BOTTOM, 1, NO_UPDATE);
      sprintf(str, "by %s", hs_name);
      text_drawf(&gold_text, str, 0, -20, A_HMIDDLE, A_BOTTOM, 1, NO_UPDATE);

      /* Don't draw menu, if quit is true */
      if(show_menu && !quit)
        menu_process_current();

      if(current_menu == &main_menu)
        {
          switch (menu_check(&main_menu))
            {
            case 2:
              done = 0;
              i = 0;
              if(level_subsets.num_items != 0)
                {
                  subset_load(&subset,level_subsets.item[0]);
                  while(!done)
                    {
                      texture_draw(&img_choose_subset,(screen->w - img_choose_subset.w) / 2, 0, NO_UPDATE);
                      if(level_subsets.num_items != 0)
                        {
                          texture_draw(&subset.image,(screen->w - subset.image.w) / 2 + 25,78,NO_UPDATE);
                          if(level_subsets.num_items > 1)
                            {
                              if(i > 0)
                                texture_draw(&arrow_left,(screen->w / 2) - ((strlen(subset.title)+2)*16)/2,20,NO_UPDATE);
                              if(i < level_subsets.num_items-1)
                                texture_draw(&arrow_right,(screen->w / 2) + ((strlen(subset.title))*16)/2,20,NO_UPDATE);
                            }
                          text_drawf(&gold_text, subset.title, 0, 20, A_HMIDDLE, A_TOP, 1, NO_UPDATE);
                          text_drawf(&gold_text, subset.description, 20, -20, A_HMIDDLE, A_BOTTOM, 1, NO_UPDATE);
                        }
                      updatescreen();
                      SDL_Delay(50);
                      while(SDL_PollEvent(&event) && !done)
                        {
                          switch(event.type)
                            {
                            case SDL_QUIT:
                              done = 1;
                              quit = 1;
                              break;
                            case SDL_KEYDOWN:		// key pressed
                              /* Keypress... */

                              key = event.key.keysym.sym;

                              if(key == SDLK_LEFT)
                                {
                                  if(i > 0)
                                    {
                                      --i;
                                      subset_free(&subset);
                                      subset_load(&subset,level_subsets.item[i]);
                                    }
                                }
                              else if(key == SDLK_RIGHT)
                                {
                                  if(i < level_subsets.num_items -1)
                                    {
                                      ++i;
                                      subset_free(&subset);
                                      subset_load(&subset,level_subsets.item[i]);
                                    }
                                }
                              else if(key == SDLK_SPACE || key == SDLK_RETURN)
                                {
                                  done = YES;
                                  quit = gameloop(subset.name,1,ST_GL_PLAY);
                                  subset_free(&subset);
                                }
                              else if(key == SDLK_ESCAPE)
                                {
                                  done = YES;
                                }
                              break;
                            default:
                              break;
                            }
                        }
                    }
                }
              break;
            case 3:
              update_load_save_game_menu(&load_game_menu, YES);
              break;
            case 5:
              done = 1;
              quit = leveleditor(1);
              break;
            case 6:
              display_credits();
              break;
            case 8:
              quit = 1;
              break;
            }
        }
      else if(current_menu == &options_menu)
        {
          process_options_menu();
        }
      else if(current_menu == &load_game_menu)
        {
          process_save_load_game_menu(NO);
        }

      flipscreen();

      /* Pause: */
      frame++;
      SDL_Delay(50);

    }
  /* Free surfaces: */

  texture_free(&bkg_title);
  texture_free(&anim1);
  texture_free(&anim2);
  string_list_free(&level_subsets);

  /* Return to main! */

  return(quit);
}

#define MAX_VEL 10
#define SPEED   1
#define SCROLL  60

void display_credits()
{
  int done;
  int scroll, speed;
  timer_type timer;
  int n,d;
  int length;
  FILE* fi;
  char temp[1024];
  string_list_type names;
  char filename[1024];
  string_list_init(&names);
  sprintf(filename,"%s/CREDITS",DATA_PREFIX);
  if((fi = fopen(filename,"r")) != NULL)
    {
      while(fgets(temp, sizeof(temp), fi) != NULL)
        {
          temp[strlen(temp)-1]='\0';
          string_list_add_item(&names,temp);
        }
      fclose(fi);
    }
    else
    {
    string_list_add_item(&names,"Credits were not found!");
    string_list_add_item(&names,"Shame on the guy, who");
    string_list_add_item(&names,"forgot to include them");
    string_list_add_item(&names,"in your SuperTux distribution.");
    }


  timer_init(&timer, SDL_GetTicks());
  timer_start(&timer, 50);

  scroll = 0;
  speed = 2;
  done = 0;

  n = d = 0;

  length = names.num_items;

  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

  while(done == 0)
    {
      /* in case of input, exit */
      while(SDL_PollEvent(&event))
        switch(event.type)
          {
          case SDL_KEYDOWN:
            switch(event.key.keysym.sym)
              {
              case SDLK_UP:
                speed -= SPEED;
                break;
              case SDLK_DOWN:
                speed += SPEED;
                break;
              case SDLK_SPACE:
              case SDLK_RETURN:
                if(speed >= 0)
                  scroll += SCROLL;
                break;
              case SDLK_ESCAPE:
                done = 1;
                break;
              default:
                break;
              }
            break;
          case SDL_QUIT:
            done = 1;
            break;
          default:
            break;
          }

      if(speed > MAX_VEL)
        speed = MAX_VEL;
      else if(speed < -MAX_VEL)
        speed = -MAX_VEL;

      /* draw the credits */

      draw_background();

      text_drawf(&white_big_text, "- Credits -", 0, screen->h-scroll, A_HMIDDLE, A_TOP, 2, NO_UPDATE);

      for(i = 0, n = 0, d = 0; i < length; i++,n++,d++)
        {
          if(names.item[i] == "")
            n--;
          else
            {
              if(names.item[i][0] == ' ')
                text_drawf(&white_small_text, names.item[i], 0, 60+screen->h+(n*18)+(d*18)-scroll-10, A_HMIDDLE, A_TOP, 1, NO_UPDATE);
              else if(names.item[i][0] == '	')
                text_drawf(&white_text, names.item[i], 0, 60+screen->h+(n*18)+(d*18)-scroll, A_HMIDDLE, A_TOP, 1, NO_UPDATE);
              else if(names.item[i+1][0] == '-' || names.item[i][0] == '-')
                text_drawf(&white_big_text, names.item[i], 0, 60+screen->h+(n*18)+(d*18)-scroll, A_HMIDDLE, A_TOP, 3, NO_UPDATE);
              else
                text_drawf(&blue_text, names.item[i], 0, 60+screen->h+(n*18)+(d*18)-scroll, A_HMIDDLE, A_TOP, 1, NO_UPDATE);
            }
        }
    
      
      texture_draw_part(&bkg_title, 0, 0, 0, 0, 640, 130, NO_UPDATE);

      flipscreen();

      if(60+screen->h+(n*18)+(d*18)-scroll < 0 && 20+60+screen->h+(n*18)+(d*18)-scroll < 0)
        done = 1;

      scroll += speed;
      if(scroll < 0)
        scroll = 0;

      SDL_Delay(35);

      if(timer_get_left(&timer) < 0)
        {
          frame++;
          timer_start(&timer, 50);
        }
    }
  string_list_free(&names);

  SDL_EnableKeyRepeat(0, 0);    // disables key repeating
  show_menu = 1;
  menu_set_current(&main_menu);
}
