/*
  title.c
  
  Super Tux - Title Screen
  
  by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/supertux/
  
  April 11, 2000 - December 9, 2003
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


/* --- TITLE SCREEN --- */

int title(void)
{
  SDL_Surface * title, * anim1, * anim2;
  SDL_Event event;
  SDLKey key;
  int done, quit, frame, pict;
  char str[80];

  game_started = 0;

  /* Init menu variables */
  initmenu();

  updatescreen();


  /* Load images: */

  title = load_image(DATA_PREFIX "/images/title/title.png", IGNORE_ALPHA);
  anim1 = load_image(DATA_PREFIX "/images/title/title-anim2.png",
                     IGNORE_ALPHA);
  anim2 = load_image(DATA_PREFIX "/images/title/title-anim1.png",
                     IGNORE_ALPHA);


  /* --- Main title loop: --- */

  done = 0;
  quit = 0;
  show_menu = 1;

  frame = 0;


  /* Draw the title background: */
  drawimage(title, 0, 0, UPDATE);


  /* Draw the high score: */
  sprintf(str, "High score: %d", load_hs());
  drawcenteredtext(str, 460, letters_red, UPDATE);

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

              if (key == SDLK_ESCAPE)
                {
                  /* Escape: Quit: */

                  quit = 1;
                }
            }
          else if (event.type == SDL_KEYUP)
            {
              key = event.key.keysym.sym;
              /* Check for menu events */
              menu_event(key);
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

      if(menu_change)
        {
          /* Draw the title background: */

          drawimage(title, 0, 0, UPDATE);

          /* Draw the high score: */
          sprintf(str, "High score: %d", load_hs());
          drawcenteredtext(str, 460, letters_red, UPDATE);
        }

      /* Don't draw menu, if quit is true */
      if(show_menu && !quit)
        quit = drawmenu();

      if(game_started)
        done = 1;

      /* Animate title screen: */

      pict = (frame / 5) % 3;

      if (pict == 0)
        drawpart(title, 560, 270, 80, 75, UPDATE);
      else if (pict == 1)
        drawimage(anim1, 560, 270, UPDATE);
      else if (pict == 2)
        drawimage(anim2, 560, 270, UPDATE);



      /* Pause: */

      SDL_Delay(50);

    }


  /* Free surfaces: */

  SDL_FreeSurface(title);
  SDL_FreeSurface(anim1);
  SDL_FreeSurface(anim2);


  /* Return to main! */

  return(quit);
}
