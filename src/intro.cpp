/*
  intro.c
  
  Super Tux - Intro Screen

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
#include "intro.h"
#include "screen.h"
#include "texture.h"
#include "timer.h"

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
  texture_type bkgd,  copter_squish,  copter_stretch, beam,
     gown_sit,  gown_lookup,  gown_upset,
     tux_sit, tux_upset, tux_mad;
  texture_type copter[2];
  SDL_Rect src, dest;
  int done, i, quit, j, scene;
  int * height, * height_speed;
  timer_type timer;
    
  /* Load sprite images: */
  texture_load(&bkgd, DATA_PREFIX "/images/intro/intro.png", IGNORE_ALPHA);  
  texture_load(&gown_sit, DATA_PREFIX "/images/intro/gown-sit.png", USE_ALPHA);
  texture_load(&gown_lookup, DATA_PREFIX "/images/intro/gown-lookup.png", USE_ALPHA);
  texture_load(&gown_upset, DATA_PREFIX "/images/intro/gown-upset.png", USE_ALPHA);
  texture_load(&tux_sit, DATA_PREFIX "/images/intro/tux-sit.png", USE_ALPHA);
  texture_load(&tux_upset, DATA_PREFIX "/images/intro/tux-upset.png", USE_ALPHA);
  texture_load(&tux_mad, DATA_PREFIX "/images/intro/tux-mad.png", USE_ALPHA);
  texture_load(&copter[0], DATA_PREFIX "/images/intro/copter1.png", USE_ALPHA);
  texture_load(&copter[1], DATA_PREFIX "/images/intro/copter2.png", USE_ALPHA); 
  texture_load(&copter_squish, DATA_PREFIX "/images/intro/copter-squish.png", USE_ALPHA); 
  texture_load(&copter_stretch, DATA_PREFIX "/images/intro/copter-stretch.png", USE_ALPHA); 
  texture_load(&beam, DATA_PREFIX "/images/intro/beam.png", USE_ALPHA); 
  
  /* Allocate buffer for height array: */
  
  height = (int*) malloc(sizeof(int) * (gown_upset.w));
  height_speed = (int*) malloc(sizeof(int) * (gown_upset.w));
  
  
  /* Initialize height arrays: */
  
  for (j = 0; j < (gown_upset.w); j++)
    {
      height[j] = 400;
      height_speed[j] = (rand() % 10) + 1;
    }
  
        /* Display background: */
  
  texture_draw_bg(&bkgd, UPDATE);
  
  /* Animation: */
  
  done = 0;
  quit = 0;
  scene = 0;
  i = 0;
  
  timer_init(&timer,NO);
  timer_start(&timer,10000);
  
  while (timer_check(&timer) && !done && !quit)
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
      
      
	      /* Display background: */
  
      /* Draw things: */
      
      if (timer_get_gone(&timer) < 2000 && scene == 0)
	{
	  ++scene;
	  /* Gown and tux sitting: */
	  
	  texture_draw(&tux_sit, 270, 400, UPDATE);
	  texture_draw(&gown_sit, 320, 400, UPDATE);
	  
	  text_drawf(&white_text, intro_text[0], 0, -8, A_HMIDDLE, A_BOTTOM, 0, NO_UPDATE);
	}
      
      
      if (timer_get_gone(&timer) >= 2000 && scene == 1)
	{
	  ++scene;
	  /* Helicopter begins to fly in: */
	  
	  erasecenteredtext(&white_text, intro_text[0], 454, &bkgd, NO_UPDATE, 1);
	  text_drawf(&white_text, intro_text[1], 0,-8, A_HMIDDLE, A_BOTTOM, 0, NO_UPDATE);
	}

      
      if (timer_get_gone(&timer) >= 2000 && timer_get_gone(&timer) < 4000)
	{
	  /* Helicopter flying in: */
	  texture_draw_part(&bkgd,0,32, 0, 32, screen->w, (copter[0].h), NO_UPDATE);
	  
	  texture_draw(&copter[i % 2],
		    (float)(timer_get_gone(&timer) - 2000) / 5  - (copter[0].w), 32,
		    NO_UPDATE);

	  update_rect(screen, 0, 32, screen->w, (copter[0].h));
	}

      
      if (timer_get_gone(&timer) >= 2500 && scene == 2)
	{
	++scene;
	  /* Gown notices something... */
	  
	  texture_draw(&gown_lookup, 320, 400, UPDATE);
	}

      
      if (timer_get_gone(&timer) >= 3500 && scene == 3)
	{
	++scene;
	  /* Gown realizes it's bad! */
	  
	  texture_draw(&gown_upset, 320, 400, UPDATE);
	}

      
      if (timer_get_gone(&timer) >= 4000 && timer_get_gone(&timer) < 8000)
	{
	  /* Helicopter sits: */
	  texture_draw_part(&bkgd,0,32, 0, 32, screen->w, (copter[0].h), NO_UPDATE);
	  
	  texture_draw(&copter[i % 2], 400 - (copter[0].w), 32, NO_UPDATE);
	  update_rect(screen, 0, 32, screen->w, (copter[0].h));
	}

      
      if (timer_get_gone(&timer) >= 5000 && scene == 4)
	{
	++scene;
	  /* Tux realizes something's happening: */
	  
	  texture_draw(&tux_upset, 270, 400, UPDATE);
	  
	  
	  erasecenteredtext(&white_text, intro_text[1], 454, &bkgd, UPDATE, 1);
	  text_drawf(&white_text, intro_text[2], 0,-8, A_HMIDDLE, A_BOTTOM, 0, NO_UPDATE);
	}
      
      
      if (timer_get_gone(&timer) >= 5000 && timer_get_gone(&timer) <= 8000)
	{
	  /* Beam gown up! */
	  
	  texture_draw_part(&bkgd,
		   310, 32 + (copter[0].h), 310,
		   32 + (copter[0].h),
		   (gown_upset.w) + 20,
		   376 + (gown_upset.h) - (copter[0].h), NO_UPDATE);
	  
	  
	  for (j = 0; j < (gown_upset.sdl_surface -> w); j++)
	    {
	      texture_draw(&beam, 320 + j - ((beam.w) / 2), height[j],
			NO_UPDATE);
	      
	      src.x = j;
	      src.y = 0;
	      src.w = 1;
	      src.h = (gown_upset.h);
	      
	      dest.x = 320 + j;
	      dest.y = height[j];
	      dest.w = src.w;
	      dest.h = src.h;
	      
	      texture_draw_part(&gown_upset,src.x,src.y,dest.x,dest.y,dest.w,dest.h,NO_UPDATE);
	      
	      height[j] = 400 + rand() % 10 - (int)(300. * ((float)(timer_get_gone(&timer) - 5000)/(float)3000.));
	      if(height[j] < 105)
	      height[j] = 105;
	    }

	  update_rect(screen,
			 310,
			 32 + (copter[0].h),
			 (gown_upset.w) + 20,
			 400 + (gown_upset.h) - (copter[0].h));
	}
      
      
      if (timer_get_gone(&timer) >= 8000 && scene == 5)
	{
		  texture_draw_part(&bkgd,
		   310, 32 + (copter[0].h), 310,
		   32 + (copter[0].h),
		   (gown_upset.w) + 20,
		   368 + (gown_upset.h) - (copter[0].h), NO_UPDATE);
	
	++scene;
	  /* Tux gets mad! */
	  
	  texture_draw(&tux_mad, 270, 400, UPDATE);
	  
	  erasecenteredtext(&white_text, intro_text[2], 454, &bkgd, UPDATE, 1);
	  text_drawf(&white_text, intro_text[3], 0,-8, A_HMIDDLE, A_BOTTOM, 0, NO_UPDATE);
	}
      
      
      if (timer_get_gone(&timer) >= 8000 && timer_get_gone(&timer) <= 8250)
	{
	  /* Helicopter starting to speed off: */
	  
	  texture_draw_part(&bkgd, 0, 32, 0, 32, screen->w, (copter_squish.h), NO_UPDATE);
	  
	  texture_draw(&copter_squish,
		    400 - (copter[0].w), 32,
		    NO_UPDATE);

	  update_rect(screen, 0, 32, screen->w, (copter_squish.h));
	}      


      if (timer_get_gone(&timer) >= 8250)
	{
	  /* Helicopter speeding off: */
	  
	  texture_draw_part(&bkgd, 0, 32, 0, 32, screen->w, (copter_stretch.h), NO_UPDATE);
	  
	  texture_draw(&copter_stretch,
		    (timer_get_gone(&timer) - 8250) /*(i - (8250 / FPS)) * 30*/ + 400 - (copter[0].w),
		    32,
		    NO_UPDATE);
		    
	  update_rect(screen, 0, 32, screen->w, (copter_stretch.h));
	}      
	
	flipscreen();

      ++i;
      /* Pause: */
      SDL_Delay(20);
    }

  
  /* Free surfaces: */
  
  texture_free(&bkgd);
  texture_free(&gown_sit);
  texture_free(&gown_lookup);
  texture_free(&gown_upset);
  texture_free(&tux_sit);
  texture_free(&tux_upset);
  texture_free(&tux_mad);
  texture_free(&copter[0]);
  texture_free(&copter[1]);
  texture_free(&copter_squish);
  texture_free(&copter_stretch);
  texture_free(&beam);
  
  
  /* Free array buffers: */
  
  free(height);
  free(height_speed);
  
  
  /* Return to main! */
  
  return(quit);
}
