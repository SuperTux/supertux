/*
  title.c
  
  Super Tux - Title Screen
  
  by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/supertux/
  
  April 11, 2000 - December 29, 2003
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

/* --- TITLE SCREEN --- */

int title(void)
{
  texture_type title, img_choose_subset, anim1, anim2;
  SDL_Event event;
  SDLKey key;
  int done, quit, frame, pict, i;
  char str[80];
  char **level_subsets;
  level_subsets = NULL;
  int subsets_num;
  level_subsets = dsubdirs("/levels", "info", &subsets_num);
  st_subset subset;
  subset_init(&subset);

  /* Rest menu variables */
  menu_reset();
  menu_set_current(&main_menu);

  clearscreen(0, 0, 0);
  updatescreen();

  /* Load images: */

  texture_load(&title,DATA_PREFIX "/images/title/title.png", IGNORE_ALPHA);
  texture_load(&anim1,DATA_PREFIX "/images/title/title-anim2.png", IGNORE_ALPHA);
  texture_load(&anim2,DATA_PREFIX "/images/title/title-anim1.png", IGNORE_ALPHA);
  texture_load(&img_choose_subset,DATA_PREFIX "/images/status/choose-level-subset.png", IGNORE_ALPHA);

  /* --- Main title loop: --- */

  done = 0;
  quit = 0;
  show_menu = 1;
  frame = 0;

  /* Draw the title background: */
  texture_draw_bg(&title, NO_UPDATE);

  /* Draw the high score: */
  load_hs();
  sprintf(str, "High score: %d", hs_score);
  text_drawf(&gold_text, str, 0, -40, A_HMIDDLE, A_BOTTOM, 1, NO_UPDATE);
  sprintf(str, "by %s", hs_name);
  text_drawf(&gold_text, str, 0, -20, A_HMIDDLE, A_BOTTOM, 1, NO_UPDATE);

  while (!done && !quit)
    {
      frame++;
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

      /* Draw the title background: */

      texture_draw_bg(&title, NO_UPDATE);

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
            case 0:
              done = 0;
              i = 0;
              subset_load(&subset,level_subsets[0]);
              while(!done)
                {
                  texture_draw(&img_choose_subset, 50, 0, NO_UPDATE);
                  if(subsets_num != 0)
                    {
                      texture_draw(&subset.image,135,78,NO_UPDATE);
                      text_drawf(&gold_text, subset.title, 0, 20, A_HMIDDLE, A_TOP, 1, NO_UPDATE);
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
                        case SDL_KEYDOWN:		// key pressed
                          /* Keypress... */

                          key = event.key.keysym.sym;

                          if(key == SDLK_LEFT)
                            {
                              if(i > 0)
                                {
                                  --i;
                                  subset_free(&subset);
                                  subset_load(&subset,level_subsets[i]);
                                }
                            }
                          else if(key == SDLK_RIGHT)
                            {
                              if(i < subsets_num -1)
                                {
                                  ++i;
                                  subset_free(&subset);
                                  subset_load(&subset,level_subsets[i]);
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
              break;
            case 3:
              done = 1;
              quit = leveleditor(1);
              break;
            case 4:
              quit = 1;
              break;
            }
        }
      else if(current_menu == &options_menu)
        {
          process_options_menu();
        }
      /* Animate title screen: */

      pict = (frame / 5) % 3;

      if (pict == 0)
        texture_draw_part(&title, 560, 270, 560, 270, 80, 75, NO_UPDATE);
      else if (pict == 1)
        texture_draw(&anim1, 560, 270, NO_UPDATE);
      else if (pict == 2)
        texture_draw(&anim2, 560, 270, NO_UPDATE);

      flipscreen();

      /* Pause: */

      SDL_Delay(50);

    }
  /* Free surfaces: */

  texture_free(&title);
  texture_free(&anim1);
  texture_free(&anim2);
  free_strings(level_subsets,subsets_num);

  /* Return to main! */

  return(quit);
}
