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
#include "type.h"


/* --- TITLE SCREEN --- */

int title(void)
{
  texture_type title, anim1, anim2;
  SDL_Event event;
  SDLKey key;
  int done, quit, frame, pict, last_highscore;
  char str[80];

  game_started = 0;
  level_editor_started = 0;

  /* Init menu variables */
  initmenu();

  clearscreen(0, 0, 0);
  updatescreen();

  /* Load images: */

  texture_load(&title,DATA_PREFIX "/images/title/title.png", IGNORE_ALPHA);
  texture_load(&anim1,DATA_PREFIX "/images/title/title-anim2.png", IGNORE_ALPHA);
  texture_load(&anim2,DATA_PREFIX "/images/title/title-anim1.png", IGNORE_ALPHA);


  /* --- Main title loop: --- */

  done = 0;
  quit = 0;
  show_menu = 1;

  frame = 0;


  /* Draw the title background: */
  texture_draw(&title, 0, 0, NO_UPDATE);


  /* Draw the high score: */
  last_highscore = load_hs();
  sprintf(str, "High score: %d", last_highscore);
  drawcenteredtext(str, 460, letters_red, NO_UPDATE, 1);

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
              menu_event(key);

              if (key == SDLK_ESCAPE)
                {
                  /* Escape: Quit: */

                  quit = 1;
                }
            }
#ifdef JOY_YES
          else if (event.type == SDL_JOYAXISMOTION)
            {
              if (event.jaxis.value > 256)
                menuaction = MN_DOWN;
              else
                menuaction = MN_UP;
            }
          else if (event.type == SDL_JOYBUTTONDOWN)
            {
              /* Joystick button: Continue: */

              menuaction = MN_HIT;
            }

#endif

        }

      if(use_gl || menu_change)
        {
          /* Draw the title background: */

          texture_draw_bg(&title, NO_UPDATE);

          /* Draw the high score: */
          sprintf(str, "High score: %d", last_highscore);
          drawcenteredtext(str, 460, letters_red, NO_UPDATE, 1);
        }

      /* Don't draw menu, if quit is true */
      if(show_menu && !quit)
        quit = drawmenu();

      if(game_started || level_editor_started)
        done = 1;

      /* Animate title screen: */

      pict = (frame / 5) % 3;

      if (pict == 0)
        texture_draw_part(&title, 560, 270, 80, 75, NO_UPDATE);
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


  /* Return to main! */

  return(quit);
}
