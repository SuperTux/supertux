/*
  intro.c
  
  Super Tux - Intro Screen
  
  by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/supertux/
  
  April 11, 2000 - April 23, 2000
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
#include "intro.h"
#include "screen.h"


char * intro_text[] = {
  "Tux and Gown were having a nice picnic..",
  "when suddenly...",
  "Gown is beamed away!!!",
  "This looks like a job for ---"
};


/* --- INTRO --- */

int intro(void)
{
  SDL_Event event;
  SDL_Surface * bkgd, * copter_squish, * copter_stretch, * beam,
    * gown_sit, * gown_lookup, * gown_upset,
    * tux_sit, * tux_upset, * tux_mad;
  SDL_Surface * copter[2];
  SDL_Rect src, dest;
  int done, i, quit, j;
  int * height, * height_speed;
  
  
  /* Load sprite images: */
  
  bkgd = load_image(DATA_PREFIX "/images/intro/intro.png", IGNORE_ALPHA);
  
  gown_sit = load_image(DATA_PREFIX "/images/intro/gown-sit.png", USE_ALPHA);
  gown_lookup = load_image(DATA_PREFIX "/images/intro/gown-lookup.png",
			   USE_ALPHA);
  gown_upset = load_image(DATA_PREFIX "/images/intro/gown-upset.png",
			  USE_ALPHA);
 
  tux_sit = load_image(DATA_PREFIX "/images/intro/tux-sit.png", USE_ALPHA);
  tux_upset = load_image(DATA_PREFIX "/images/intro/tux-upset.png",
			 USE_ALPHA);
  tux_mad = load_image(DATA_PREFIX "/images/intro/tux-mad.png", USE_ALPHA);
 
  copter[0] = load_image(DATA_PREFIX "/images/intro/copter1.png", USE_ALPHA);
  copter[1] = load_image(DATA_PREFIX "/images/intro/copter2.png", USE_ALPHA);

  copter_squish = load_image(DATA_PREFIX "/images/intro/copter-squish.png",
			     USE_ALPHA);
  copter_stretch = load_image(DATA_PREFIX "/images/intro/copter-stretch.png",
			      USE_ALPHA);
 
  beam = load_image(DATA_PREFIX "/images/intro/beam.png", USE_ALPHA);
  
  
  /* Allocate buffer for height array: */
  
  height = malloc(sizeof(int) * (gown_upset -> w));
  height_speed = malloc(sizeof(int) * (gown_upset -> w));
  
  
  /* Initialize height arrays: */
  
  for (j = 0; j < (gown_upset -> w); j++)
    {
      height[j] = 400;
      height_speed[j] = (rand() % 10) + 1;
    }
  
  
  /* Display background: */
  
  drawimage(bkgd, 0, 0, UPDATE);
  
  
  /* Animation: */
  
  done = 0;
  quit = 0;
  
  for (i = 0; i < (10000 / FPS) && !done && !quit; i++)
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
	      /* Keypress - skip intro: */
	      
	      done = 1;
	    }
#ifdef JOY_YES
	  else if (event.type == SDL_JOYBUTTONDOWN)
	    {
	      /* Fire button - skip intro: */
	      
	      done = 1;
	    }
#endif
	}
      
      
      /* Draw things: */
      
      if (i == 0)
	{
	  /* Gown and tux sitting: */
	  
	  drawimage(tux_sit, 270, 400, UPDATE);
	  drawimage(gown_sit, 320, 400, UPDATE);
	  
	  drawcenteredtext(intro_text[0], 456, letters_blue, UPDATE);
	}
      
      
      if (i == (2000 / FPS))
	{
	  /* Helicopter begins to fly in: */
	  
	  erasecenteredtext(intro_text[0], 456, bkgd, UPDATE);
	  drawcenteredtext(intro_text[1], 456, letters_red, UPDATE);
	}

      
      if (i > (2000 / FPS) && i < (4000 / FPS))
	{
	  /* Helicopter flying in: */
	  
	  drawpart(bkgd, 0, 32, 640, (copter[0] -> h), NO_UPDATE);
	  
	  drawimage(copter[i % 2],
		    (i - (2000 / FPS)) * (FPS / 5) - (copter[0] -> w), 32,
		    NO_UPDATE);
	  
	  SDL_UpdateRect(screen, 0, 32, 640, (copter[0] -> h));
	}

      
      if (i == (2500 / FPS))
	{
	  /* Gown notices something... */
	  
	  drawimage(gown_lookup, 320, 400, UPDATE);
	}

      
      if (i == (3500 / FPS))
	{
	  /* Gown realizes it's bad! */
	  
	  drawimage(gown_upset, 320, 400, UPDATE);
	}

      
      if (i > (4000 / FPS) && i < (8000 / FPS))
	{
	  /* Helicopter sits: */
	  
	  drawpart(bkgd, 0, 32, 640, (copter[0] -> h), NO_UPDATE);
	  
	  drawimage(copter[i % 2], 400 - (copter[0] -> w), 32, NO_UPDATE);

	  SDL_UpdateRect(screen, 0, 32, 640, (copter[0] -> h));
	}

      
      if (i == (5000 / FPS))
	{
	  /* Tux realizes something's happening: */
	  
	  drawimage(tux_upset, 270, 400, UPDATE);
	  
	  
	  erasecenteredtext(intro_text[1], 456, bkgd, UPDATE);
	  drawcenteredtext(intro_text[2], 456, letters_red, UPDATE);
	}
      
      
      if (i > (5000 / FPS))
	{
	  /* Beam gown up! */
	  
	  drawpart(bkgd,
		   320,
		   32 + (copter[0] -> h),
		   (gown_upset -> w),
		   368 + (gown_upset -> h) - (copter[0] -> h), NO_UPDATE);
	  
	  
	  for (j = 0; j < (gown_upset -> w); j++)
	    {
	      drawimage(beam, 320 + j - ((beam -> w) / 2), height[j],
			NO_UPDATE);
	      
	      src.x = j;
	      src.y = 0;
	      src.w = 1;
	      src.h = (gown_upset -> h);
	      
	      dest.x = 320 + j;
	      dest.y = height[j];
	      dest.w = src.w;
	      dest.h = src.h;
	      
	      SDL_BlitSurface(gown_upset, &src, screen, &dest);
	      
	      height[j] = height[j] - height_speed[j];
	      
	      if ((i % 2) == 0)
		height_speed[j]++;
	    }
	  
	  SDL_UpdateRect(screen,
			 320,
			 32 + (copter[0] -> h),
			 (gown_upset -> w),
			 400 + (gown_upset -> h) - (copter[0] -> h));
	}
      
      
      if (i == (8000 / FPS))
	{
	  /* Tux gets mad! */
	  
	  drawimage(tux_mad, 270, 400, UPDATE);
	  
	  erasecenteredtext(intro_text[2], 456, bkgd, UPDATE);
	  drawcenteredtext(intro_text[3], 456, letters_gold, UPDATE);
	}
      
      
      if (i > (8000 / FPS) && i < (8250 / FPS))
	{
	  /* Helicopter starting to speed off: */
	  
	  drawpart(bkgd, 0, 32, 640, (copter_squish -> h), NO_UPDATE);
	  
	  drawimage(copter_squish,
		    400 - (copter[0] -> w), 32,
		    NO_UPDATE);
	  
	  SDL_UpdateRect(screen, 0, 32, 640, (copter_squish -> h));
	}      


      if (i > (8250 / FPS))
	{
	  /* Helicopter speeding off: */
	  
	  drawpart(bkgd, 0, 32, 640, (copter_stretch -> h), NO_UPDATE);
	  
	  drawimage(copter_stretch,
		    (i - (8250 / FPS)) * 30 + 400 - (copter[0] -> w),
		    32,
		    NO_UPDATE);
	  
	  SDL_UpdateRect(screen, 0, 32, 640, (copter_stretch -> h));
	}      
      
      /* Pause: */
      
      SDL_Delay(FPS);
    }

  
  /* Free surfaces: */
  
  SDL_FreeSurface(bkgd);
  
  SDL_FreeSurface(gown_sit);
  SDL_FreeSurface(gown_lookup);
  SDL_FreeSurface(gown_upset);
  
  SDL_FreeSurface(tux_sit);
  SDL_FreeSurface(tux_upset);
  SDL_FreeSurface(tux_mad);
  
  SDL_FreeSurface(copter[0]);
  SDL_FreeSurface(copter[1]);

  SDL_FreeSurface(copter_squish);
  SDL_FreeSurface(copter_stretch);

  SDL_FreeSurface(beam);
  
  
  /* Free array buffers: */
  
  free(height);
  free(height_speed);
  
  
  /* Return to main! */
  
  return(quit);
}
