/*
  gameloop.c
  
  Super Tux - Game Loop!
  
  by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/supertux/
  
  April 11, 2000 - January 1st, 2004
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <SDL.h>

#ifdef LINUX
#include <pwd.h>
#include <sys/types.h>
#include <ctype.h>
#endif

#include "defines.h"
#include "globals.h"
#include "gameloop.h"
#include "screen.h"
#include "setup.h"
#include "high_scores.h"
#include "menu.h"
#include "badguy.h"
#include "world.h"
#include "special.h"
#include "player.h"
#include "level.h"
#include "scene.h"
#include "collision.h"

/* extern variables */

extern char* soundfilenames[NUM_SOUNDS];

/* Local variables: */

texture_type img_waves[3], img_water, img_pole, img_poletop, img_flag[2];
texture_type img_cloud[2][4];
SDL_Event event;
SDLKey key;
char level_subset[100];
char str[60];
timer_type time_left;

/* Local function prototypes: */

void levelintro(void);
void initgame(void);
void loadlevelsong(void);
void unloadlevelsong(void);
void loadshared(void);
void unloadshared(void);
void drawstatus(void);
void drawendscreen(void);
void drawresultscreen(void);

void levelintro(void)
{
  /* Level Intro: */

  clearscreen(0, 0, 0);

  sprintf(str, "LEVEL %d", level);
  drawcenteredtext(str, 200, letters_red, NO_UPDATE, 1);

  sprintf(str, "%s", current_level.name);
  drawcenteredtext(str, 224, letters_gold, NO_UPDATE, 1);

  sprintf(str, "TUX x %d", tux.lives);
  drawcenteredtext(str, 256, letters_blue, NO_UPDATE, 1);

  flipscreen();

  SDL_Delay(1000);
}

/* Reset Timers */
void start_timers(void)
{
  timer_start(&time_left,current_level.time_left*1000);
}

void activate_bad_guys(void)
{
  int x,y;

  /* Activate bad guys: */

  for (y = 0; y < 15; y++)
    {
      for (x = 0; x < current_level.width; x++)
        {
          if (current_level.tiles[y][x] >= '0' && current_level.tiles[y][x] <= '9')
            {
              add_bad_guy(x * 32, y * 32, current_level.tiles[y][x] - '0');
              current_level.tiles[y][x] = '.';
            }
        }
    }

}

/* --- GAME EVENT! --- */

void game_event(void)
{
  while (SDL_PollEvent(&event))
    {
    switch(event.type)
      {
      case SDL_QUIT:        /* Quit event - quit: */
          quit = 1;
          break;
      case SDL_KEYDOWN:     /* A keypress! */
          key = event.key.keysym.sym;

          /* Check for menu-events, if the menu is shown */
          if(show_menu)
            menu_event(key);

          switch(key)
            {
            case SDLK_ESCAPE:    /* Escape: Open/Close the menu: */
              if(!game_pause)
                {
                  if(show_menu)
                    show_menu = 0;
                  else
                    show_menu = 1;
                }
               break;
          case SDLK_RIGHT:
              tux.input.right = DOWN;
               break;
          case SDLK_LEFT:
              tux.input.left = DOWN;
            break;
          case SDLK_UP:
              tux.input.up = DOWN;
            break;
          case SDLK_DOWN:
              tux.input.down = DOWN;
            break;
          case SDLK_LCTRL:
              tux.input.fire = DOWN;
            break;
           default:
            break;
        }
        break;
      case SDL_KEYUP:      /* A keyrelease! */
          key = event.key.keysym.sym;

          switch(key)
           {
          case SDLK_RIGHT:
              tux.input.right = UP;
            break;
          case SDLK_LEFT:
              tux.input.left = UP;
            break;
          case SDLK_UP:
              tux.input.up = UP;
            break;
          case SDLK_DOWN:
              tux.input.down = UP;
            break;
          case SDLK_LCTRL:
              tux.input.fire = UP;
            break;
          case SDLK_p:
              if(!show_menu)
                {
                  if(game_pause)
                    game_pause = 0;
                  else
                    game_pause = 1;
                }
            break;
          case SDLK_TAB:
            if(debug_mode == YES)
              tux.size = !tux.size;
            break;
          case SDLK_END:
            if(debug_mode == YES)
              distros += 50;
            break;
          case SDLK_SPACE:
            if(debug_mode == YES)
              next_level = 1;
            break;
          case SDLK_DELETE:
            if(debug_mode == YES)
              tux.got_coffee = 1;
            break;
          case SDLK_INSERT:
            if(debug_mode == YES)
              timer_start(&tux.invincible_timer,TUX_INVINCIBLE_TIME);
            break;
           default:
            break;
        }
        break;
#ifdef JOY_YES
      case SDL_JOYAXISMOTION:
        switch(event.jaxis.axis)
           {
           case JOY_X:
              if (event.jaxis.value < -256)
                tux.input.left = DOWN;
              else
                tux.input.left = UP;

              if (event.jaxis.value > 256)
                tux.input.right = DOWN;
              else
                tux.input.right = UP;
            break;
          case JOY_Y:
              if (event.jaxis.value > 256)
                tux.input.down = DOWN;
              else
                tux.input.down = UP;

              /* Handle joystick for the menu */
              if(show_menu)
                {
                  if(tux.input.down == DOWN)
                    menuaction = MN_DOWN;
                  else
                    menuaction = MN_UP;
                }
            break;
            default:
              break;
        }
        break;
      case SDL_JOYBUTTONDOWN:
          if (event.jbutton.button == JOY_A)
            tux.input.up = DOWN;
          else if (event.jbutton.button == JOY_B)
            tux.input.fire = DOWN;
        break;
      case SDL_JOYBUTTONUP:
          if (event.jbutton.button == JOY_A)
            tux.input.up = UP;
          else if (event.jbutton.button == JOY_B)
            tux.input.fire = UP;

          if(show_menu)
            menuaction = MN_HIT;
          break;
       default:
         break;

        }
#endif

    }

}

/* --- GAME ACTION! --- */

int game_action(void)
{
int i;

  /* (tux_dying || next_level) */
  if (tux.dying || next_level)
    {
      /* Tux either died, or reached the end of a level! */


      if (playing_music())
        halt_music();


      if (next_level)
        {
          /* End of a level! */
          level++;
          next_level = 0;
          drawresultscreen();
        }
      else
        {

	player_dying(&tux);
	
          /* No more lives!? */

          if (tux.lives < 0)
            {
              drawendscreen();

              if (score > highscore)
                save_hs(score);
              unloadlevelgfx();
              unloadlevelsong();
              unloadshared();
              return(0);
            } /* if (lives < 0) */
        }

      /* Either way, (re-)load the (next) level... */

      player_level_begin(&tux);
      set_defaults();
      loadlevel(&current_level,"default",level);
      activate_bad_guys();
      unloadlevelgfx();
      loadlevelgfx(&current_level);
      unloadlevelsong();
      loadlevelsong();
      levelintro();
      start_timers();
    }

  player_action(&tux);

  /* Handle bouncy distros: */

  for (i = 0; i < NUM_BOUNCY_DISTROS; i++)
    {
      bouncy_distro_action(&bouncy_distros[i]);
    }


  /* Handle broken bricks: */

  for (i = 0; i < NUM_BROKEN_BRICKS; i++)
    {
      broken_brick_action(&broken_bricks[i]);
    }


  /* Handle distro counting: */

  if (counting_distros == YES)
    {
      distro_counter--;

      if (distro_counter <= 0)
        counting_distros = -1;
    }


  /* Handle bouncy bricks: */

  for (i = 0; i < NUM_BOUNCY_BRICKS; i++)
    {
      bouncy_brick_action(&bouncy_bricks[i]);
    }


  /* Handle floating scores: */

  for (i = 0; i < NUM_FLOATING_SCORES; i++)
    {
      floating_score_action(&floating_scores[i]);
    }


  /* Handle bullets: */

  for (i = 0; i < NUM_BULLETS; ++i)
    {
      bullet_action(&bullets[i]);
    }


  /* Handle background timer: */

  if (super_bkgd_time)
    super_bkgd_time--;


  /* Handle upgrades: */

  for (i = 0; i < NUM_UPGRADES; i++)
    {
	upgrade_action(&upgrades[i]);
    } /* for (i = 0; i < NUM_UPGRADES; i++) */


  /* Handle bad guys: */

  for (i = 0; i < NUM_BAD_GUYS; i++)
    {
	badguy_action(&bad_guys[i]);
    }

  /* Handle all possible collisions. */
  collision_handler();

  return -1;
}

/* --- GAME DRAW! --- */

void game_draw()
{
  int  x, y, i;

  /* Draw screen: */

  if (tux.dying && (frame % 4) == 0)
    clearscreen(255, 255, 255);
  else
    {
      if (super_bkgd_time == 0)
        clearscreen(current_level.bkgd_red, current_level.bkgd_green, current_level.bkgd_blue);
      else
        texture_draw(&img_super_bkgd, 0, 0, NO_UPDATE);
    }

  /* Draw background: */

  for (y = 0; y < 15; y++)
    {
      for (x = 0; x < 21; x++)
        {
          drawshape(x * 32 - ((int)scroll_x % 32), y * 32,
                    current_level.tiles[(int)y][(int)x + (int)(scroll_x / 32)]);
        }
    }


  /* (Bouncy bricks): */

  for (i = 0; i < NUM_BOUNCY_BRICKS; i++)
    {
	bouncy_brick_draw(&bouncy_bricks[i]);
    }


  /* (Bad guys): */

  for (i = 0; i < NUM_BAD_GUYS; i++)
    {
	badguy_draw(&bad_guys[i]);
    }

  /* (Tux): */
  
  player_draw(&tux);

  /* (Bullets): */

  for (i = 0; i < NUM_BULLETS; i++)
    {
       bullet_draw(&bullets[i]);
    }

  /* (Floating scores): */

  for (i = 0; i < NUM_FLOATING_SCORES; i++)
    {
	floating_score_draw(&floating_scores[i]);
    }


  /* (Upgrades): */

  for (i = 0; i < NUM_UPGRADES; i++)
    {
	upgrade_draw(&upgrades[i]);
    }


  /* (Bouncy distros): */

  for (i = 0; i < NUM_BOUNCY_DISTROS; i++)
    {
      bouncy_distro_draw(&bouncy_distros[i]);
    }


  /* (Broken bricks): */

  for (i = 0; i < NUM_BROKEN_BRICKS; i++)
    {
	broken_brick_draw(&broken_bricks[i]);
    }

  drawstatus();


  if(game_pause)
    drawcenteredtext("PAUSE",230,letters_red, NO_UPDATE, 1);

  if(show_menu)
    done = drawmenu();

  /* (Update it all!) */

  updatescreen();


}

/* --- GAME LOOP! --- */

int gameloop(void)
{

  Uint32 last_time, now_time;

  /* Clear screen: */

  clearscreen(0, 0, 0);
  updatescreen();


  /* Init the game: */

  initmenu();
  menumenu = MENU_GAME;
  initgame();
  loadshared();
  set_defaults();
  
  loadlevel(&current_level,"default",level);
  loadlevelgfx(&current_level);
  activate_bad_guys();
  loadlevelsong();
  highscore = load_hs();

  player_init(&tux);
  
  levelintro();
  start_timers();
    
  /* --- MAIN GAME LOOP!!! --- */

  done = 0;
  quit = 0;
  frame = 0;
  game_pause = 0;
  
  game_draw();
  do
    {
      last_time = SDL_GetTicks();
      frame++;


      /* Handle events: */

      tux.input.old_fire = tux.input.fire;

      game_event();

     
      /* Handle actions: */

      if(!game_pause && !show_menu)
        {
          if (game_action() == 0)
            {
              /* == 0: no more lives */
              /* == -1: continues */
              return 0;
            }
        }
      else
        SDL_Delay(50);

      /*Draw the current scene to the screen */
      game_draw();

      /* Time stops in pause mode */
      if(game_pause || show_menu )
        {
          continue;
        }

      /* Pause til next frame: */

      now_time = SDL_GetTicks();
      /*if (now_time < last_time + FPS)
        SDL_Delay(last_time + FPS - now_time);*/
	SDL_Delay(10);


      /* Handle time: */

      if (timer_check(&time_left))
        {
	      /* are we low on time ? */
      if ((timer_get_left(&time_left) < TIME_WARNING)
          && (current_music != HURRYUP_MUSIC))
        {
          current_music = HURRYUP_MUSIC;
          /* stop the others music, prepare to play the fast music */
          if (playing_music())
            {
              halt_music();
            }
        }

        }
	else
	player_kill(&tux,KILL);
	

      /* Keep playing the correct music: */

      if (!playing_music())
        {
	   play_current_music();
        }

	
    }
  while (!done && !quit);

  if (playing_music())
    halt_music();

  unloadlevelgfx();
  unloadlevelsong();
  unloadshared();

  return(quit);
}


/* Initialize the game stuff: */

void initgame(void)
{
  level = 1;
  score = 0;
  distros = 0;
}

/* Free music data for this level: */

void unloadlevelsong(void)
{
  free_music(level_song);
  free_music(level_song_fast);
}

/* Load music: */

void loadlevelsong(void)
{

  char * song_path;
  char * song_subtitle;

  song_path = (char *) malloc(sizeof(char) * (strlen(DATA_PREFIX) +
                              strlen(current_level.song_title) + 8));
  sprintf(song_path, "%s/music/%s", DATA_PREFIX, current_level.song_title);
  level_song = load_song(song_path);
  free(song_path);

  
  song_path = (char *) malloc(sizeof(char) * (strlen(DATA_PREFIX) +
                              strlen(current_level.song_title) + 8 + 5));
  song_subtitle = strdup(current_level.song_title);
  strcpy(strstr(song_subtitle, "."), "\0");
  sprintf(song_path, "%s/music/%s-fast%s", DATA_PREFIX, song_subtitle, strstr(current_level.song_title, "."));
  level_song_fast = load_song(song_path);
  free(song_subtitle);
  free(song_path);
}

/* Load graphics/sounds shared between all levels: */

void loadshared(void)
{
  int i;
  char * herring_song_path; /* for loading herring song*/

  /* Tuxes: */

  texture_load(&tux_right[0],DATA_PREFIX "/images/shared/tux-right-0.png", USE_ALPHA);
  texture_load(&tux_right[1],DATA_PREFIX "/images/shared/tux-right-1.png", USE_ALPHA);
  texture_load(&tux_right[2],DATA_PREFIX "/images/shared/tux-right-2.png", USE_ALPHA);  

  texture_load(&tux_left[0],DATA_PREFIX "/images/shared/tux-left-0.png", USE_ALPHA);
  texture_load(&tux_left[1],DATA_PREFIX "/images/shared/tux-left-1.png", USE_ALPHA);
  texture_load(&tux_left[2],DATA_PREFIX "/images/shared/tux-left-2.png", USE_ALPHA);  
  
  texture_load(&firetux_right[0],DATA_PREFIX "/images/shared/firetux-right-0.png", USE_ALPHA);  
  texture_load(&firetux_right[1],DATA_PREFIX "/images/shared/firetux-right-1.png", USE_ALPHA); 
  texture_load(&firetux_right[2],DATA_PREFIX "/images/shared/firetux-right-2.png", USE_ALPHA); 

  texture_load(&firetux_left[0],DATA_PREFIX "/images/shared/firetux-left-0.png", USE_ALPHA);  
  texture_load(&firetux_left[1],DATA_PREFIX "/images/shared/firetux-left-1.png", USE_ALPHA); 
  texture_load(&firetux_left[2],DATA_PREFIX "/images/shared/firetux-left-2.png", USE_ALPHA); 


  texture_load(&cape_right[0] ,DATA_PREFIX "/images/shared/cape-right-0.png",
                             USE_ALPHA);

  texture_load(&cape_right[1] ,DATA_PREFIX "/images/shared/cape-right-1.png",
                             USE_ALPHA);

  texture_load(&cape_left[0] ,DATA_PREFIX "/images/shared/cape-left-0.png",
                            USE_ALPHA);

  texture_load(&cape_left[1] ,DATA_PREFIX "/images/shared/cape-left-1.png",
                            USE_ALPHA);

  texture_load(&bigtux_right[0] ,DATA_PREFIX "/images/shared/bigtux-right-0.png",
                               USE_ALPHA);

  texture_load(&bigtux_right[1] ,DATA_PREFIX "/images/shared/bigtux-right-1.png",
                               USE_ALPHA);

  texture_load(&bigtux_right[2] ,DATA_PREFIX "/images/shared/bigtux-right-2.png",
                               USE_ALPHA);

  texture_load(&bigtux_right_jump ,DATA_PREFIX "/images/shared/bigtux-right-jump.png", USE_ALPHA);

  texture_load(&bigtux_left[0] ,DATA_PREFIX "/images/shared/bigtux-left-0.png",
                              USE_ALPHA);

  texture_load(&bigtux_left[1] ,DATA_PREFIX "/images/shared/bigtux-left-1.png",
                              USE_ALPHA);

  texture_load(&bigtux_left[2] ,DATA_PREFIX "/images/shared/bigtux-left-2.png",
                              USE_ALPHA);

  texture_load(&bigtux_left_jump ,DATA_PREFIX "/images/shared/bigtux-left-jump.png", USE_ALPHA);

  texture_load(&bigcape_right[0] ,DATA_PREFIX "/images/shared/bigcape-right-0.png",
               USE_ALPHA);

  texture_load(&bigcape_right[1] ,DATA_PREFIX "/images/shared/bigcape-right-1.png",
               USE_ALPHA);

  texture_load(&bigcape_left[0] ,DATA_PREFIX "/images/shared/bigcape-left-0.png",
               USE_ALPHA);

  texture_load(&bigcape_left[1] ,DATA_PREFIX "/images/shared/bigcape-left-1.png",
               USE_ALPHA);

  texture_load(&bigfiretux_right[0] ,DATA_PREFIX "/images/shared/bigfiretux-right-0.png",
                                   USE_ALPHA);

  texture_load(&bigfiretux_right[1] ,DATA_PREFIX "/images/shared/bigfiretux-right-1.png",
                                   USE_ALPHA);

  texture_load(&bigfiretux_right[2] ,DATA_PREFIX "/images/shared/bigfiretux-right-2.png",
                                   USE_ALPHA);

  texture_load(&bigfiretux_right_jump ,DATA_PREFIX "/images/shared/bigfiretux-right-jump.png", USE_ALPHA);

  texture_load(&bigfiretux_left[0] ,DATA_PREFIX "/images/shared/bigfiretux-left-0.png",
                                  USE_ALPHA);

  texture_load(&bigfiretux_left[1] ,DATA_PREFIX "/images/shared/bigfiretux-left-1.png",
                                  USE_ALPHA);

  texture_load(&bigfiretux_left[2] ,DATA_PREFIX "/images/shared/bigfiretux-left-2.png",
                                  USE_ALPHA);

  texture_load(&bigfiretux_left_jump ,DATA_PREFIX "/images/shared/bigfiretux-left-jump.png", USE_ALPHA);

  texture_load(&bigcape_right[0] ,DATA_PREFIX "/images/shared/bigcape-right-0.png",
               USE_ALPHA);

  texture_load(&bigcape_right[1] ,DATA_PREFIX "/images/shared/bigcape-right-1.png",
               USE_ALPHA);

  texture_load(&bigcape_left[0] ,DATA_PREFIX "/images/shared/bigcape-left-0.png",
               USE_ALPHA);

  texture_load(&bigcape_left[1] ,DATA_PREFIX "/images/shared/bigcape-left-1.png",
               USE_ALPHA);


  texture_load(&ducktux_right ,DATA_PREFIX
                             "/images/shared/ducktux-right.png",
                             USE_ALPHA);

  texture_load(&ducktux_left ,DATA_PREFIX
                            "/images/shared/ducktux-left.png",
                            USE_ALPHA);

  texture_load(&skidtux_right ,DATA_PREFIX
                             "/images/shared/skidtux-right.png",
                             USE_ALPHA);

  texture_load(&skidtux_left ,DATA_PREFIX
                            "/images/shared/skidtux-left.png",
                            USE_ALPHA);

  texture_load(&duckfiretux_right ,DATA_PREFIX
                                 "/images/shared/duckfiretux-right.png",
                                 USE_ALPHA);

  texture_load(&duckfiretux_left ,DATA_PREFIX
                                "/images/shared/duckfiretux-left.png",
                                USE_ALPHA);

  texture_load(&skidfiretux_right ,DATA_PREFIX
                                 "/images/shared/skidfiretux-right.png",
                                 USE_ALPHA);

  texture_load(&skidfiretux_left ,DATA_PREFIX
                                "/images/shared/skidfiretux-left.png",
                                USE_ALPHA);


  /* Boxes: */

  texture_load(&img_box_full ,DATA_PREFIX "/images/shared/box-full.png",
                            IGNORE_ALPHA);
  texture_load(&img_box_empty ,DATA_PREFIX "/images/shared/box-empty.png",
                             IGNORE_ALPHA);


  /* Water: */


  texture_load(&img_water ,DATA_PREFIX "/images/shared/water.png", IGNORE_ALPHA);

  texture_load(&img_waves[0] ,DATA_PREFIX "/images/shared/waves-0.png",
                            USE_ALPHA);

  texture_load(&img_waves[1] ,DATA_PREFIX "/images/shared/waves-1.png",
                            USE_ALPHA);

  texture_load(&img_waves[2] ,DATA_PREFIX "/images/shared/waves-2.png",
                            USE_ALPHA);


  /* Pole: */

  texture_load(&img_pole ,DATA_PREFIX "/images/shared/pole.png", USE_ALPHA);
  texture_load(&img_poletop ,DATA_PREFIX "/images/shared/poletop.png",
                           USE_ALPHA);


  /* Flag: */

  texture_load(&img_flag[0] ,DATA_PREFIX "/images/shared/flag-0.png",
                           USE_ALPHA);
  texture_load(&img_flag[1] ,DATA_PREFIX "/images/shared/flag-1.png",
                           USE_ALPHA);


  /* Cloud: */

  texture_load(&img_cloud[0][0] ,DATA_PREFIX "/images/shared/cloud-00.png",
                               USE_ALPHA);

  texture_load(&img_cloud[0][1] ,DATA_PREFIX "/images/shared/cloud-01.png",
                               USE_ALPHA);

  texture_load(&img_cloud[0][2] ,DATA_PREFIX "/images/shared/cloud-02.png",
                               USE_ALPHA);

  texture_load(&img_cloud[0][3] ,DATA_PREFIX "/images/shared/cloud-03.png",
                               USE_ALPHA);


  texture_load(&img_cloud[1][0] ,DATA_PREFIX "/images/shared/cloud-10.png",
                               USE_ALPHA);

  texture_load(&img_cloud[1][1] ,DATA_PREFIX "/images/shared/cloud-11.png",
                               USE_ALPHA);

  texture_load(&img_cloud[1][2] ,DATA_PREFIX "/images/shared/cloud-12.png",
                               USE_ALPHA);

  texture_load(&img_cloud[1][3] ,DATA_PREFIX "/images/shared/cloud-13.png",
                               USE_ALPHA);


  /* Bad guys: */

  /* (BSOD) */

  texture_load(&img_bsod_left[0] ,DATA_PREFIX
                                "/images/shared/bsod-left-0.png",
                                USE_ALPHA);

  texture_load(&img_bsod_left[1] ,DATA_PREFIX
                                "/images/shared/bsod-left-1.png",
                                USE_ALPHA);

  texture_load(&img_bsod_left[2] ,DATA_PREFIX
                                "/images/shared/bsod-left-2.png",
                                USE_ALPHA);

  texture_load(&img_bsod_left[3] ,DATA_PREFIX
                                "/images/shared/bsod-left-3.png",
                                USE_ALPHA);

  texture_load(&img_bsod_right[0] ,DATA_PREFIX
                                 "/images/shared/bsod-right-0.png",
                                 USE_ALPHA);

  texture_load(&img_bsod_right[1] ,DATA_PREFIX
                                 "/images/shared/bsod-right-1.png",
                                 USE_ALPHA);

  texture_load(&img_bsod_right[2] ,DATA_PREFIX
                                 "/images/shared/bsod-right-2.png",
                                 USE_ALPHA);

  texture_load(&img_bsod_right[3] ,DATA_PREFIX
                                 "/images/shared/bsod-right-3.png",
                                 USE_ALPHA);

  texture_load(&img_bsod_squished_left ,DATA_PREFIX
                                      "/images/shared/bsod-squished-left.png",
                                      USE_ALPHA);

  texture_load(&img_bsod_squished_right ,DATA_PREFIX
                                       "/images/shared/bsod-squished-right.png",
                                       USE_ALPHA);

  texture_load(&img_bsod_falling_left ,DATA_PREFIX
                                     "/images/shared/bsod-falling-left.png",
                                     USE_ALPHA);

  texture_load(&img_bsod_falling_right ,DATA_PREFIX
                                      "/images/shared/bsod-falling-right.png",
                                      USE_ALPHA);


  /* (Laptop) */

  texture_load(&img_laptop_left[0] ,DATA_PREFIX
                                  "/images/shared/laptop-left-0.png",
                                  USE_ALPHA);

  texture_load(&img_laptop_left[1] ,DATA_PREFIX
                                  "/images/shared/laptop-left-1.png",
                                  USE_ALPHA);

  texture_load(&img_laptop_left[2] ,DATA_PREFIX
                                  "/images/shared/laptop-left-2.png",
                                  USE_ALPHA);

  texture_load(&img_laptop_right[0] ,DATA_PREFIX
                                   "/images/shared/laptop-right-0.png",
                                   USE_ALPHA);

  texture_load(&img_laptop_right[1] ,DATA_PREFIX
                                   "/images/shared/laptop-right-1.png",
                                   USE_ALPHA);

  texture_load(&img_laptop_right[2] ,DATA_PREFIX
                                   "/images/shared/laptop-right-2.png",
                                   USE_ALPHA);

  texture_load(&img_laptop_flat_left ,DATA_PREFIX
                                    "/images/shared/laptop-flat-left.png",
                                    USE_ALPHA);

  texture_load(&img_laptop_flat_right ,DATA_PREFIX
                                     "/images/shared/laptop-flat-right.png",
                                     USE_ALPHA);

  texture_load(&img_laptop_falling_left ,DATA_PREFIX
               "/images/shared/laptop-falling-left.png",
               USE_ALPHA);

  texture_load(&img_laptop_falling_right ,DATA_PREFIX
               "/images/shared/laptop-falling-right.png",
               USE_ALPHA);


  /* (Money) */

  texture_load(&img_money_left[0] ,DATA_PREFIX
                                 "/images/shared/bag-left-0.png",
                                 USE_ALPHA);

  texture_load(&img_money_left[1] ,DATA_PREFIX
                                 "/images/shared/bag-left-1.png",
                                 USE_ALPHA);

  texture_load(&img_money_right[0] ,DATA_PREFIX
                                  "/images/shared/bag-right-0.png",
                                  USE_ALPHA);

  texture_load(&img_money_right[1] ,DATA_PREFIX
                                  "/images/shared/bag-right-1.png",
                                  USE_ALPHA);



  /* Upgrades: */

  texture_load(&img_mints ,DATA_PREFIX "/images/shared/mints.png", USE_ALPHA);
  texture_load(&img_coffee ,DATA_PREFIX "/images/shared/coffee.png", USE_ALPHA);


  /* Weapons: */

  texture_load(&img_bullet ,DATA_PREFIX "/images/shared/bullet.png", USE_ALPHA);

  texture_load(&img_red_glow ,DATA_PREFIX "/images/shared/red-glow.png",
                            USE_ALPHA);



  /* Distros: */

  texture_load(&img_distro[0] ,DATA_PREFIX "/images/shared/distro-0.png",
                             USE_ALPHA);

  texture_load(&img_distro[1] ,DATA_PREFIX "/images/shared/distro-1.png",
                             USE_ALPHA);

  texture_load(&img_distro[2] ,DATA_PREFIX "/images/shared/distro-2.png",
                             USE_ALPHA);

  texture_load(&img_distro[3] ,DATA_PREFIX "/images/shared/distro-3.png",
                             USE_ALPHA);


  /* Tux life: */

  texture_load(&tux_life ,DATA_PREFIX "/images/shared/tux-life.png",
                        USE_ALPHA);

  /* Herring: */

  texture_load(&img_golden_herring, DATA_PREFIX "/images/shared/golden-herring.png",
               USE_ALPHA);


  /* Super background: */

  texture_load(&img_super_bkgd ,DATA_PREFIX "/images/shared/super-bkgd.png",
                              IGNORE_ALPHA);


  /* Sound effects: */

  /* if (use_sound) // this will introduce SERIOUS bugs here ! because "load_sound"
                    // initialize sounds[i] with the correct pointer's value:
                    // NULL or something else. And it will be dangerous to
                    // play with not-initialized pointers.
                    // This is also true with if (use_music)
     Send a mail to me: neoneurone@users.sf.net, if you have another opinion. :)
  */
  for (i = 0; i < NUM_SOUNDS; i++)
    sounds[i] = load_sound(soundfilenames[i]);

  /* Herring song */
  herring_song_path = (char *) malloc(sizeof(char) * (strlen(DATA_PREFIX) +
                                      strlen("SALCON.MOD") + 8)); /* FIXME: We need a real herring_song! Thats a fake.:) */

  sprintf(herring_song_path, "%s/music/%s", DATA_PREFIX, "SALCON.MOD");

  herring_song = load_song(herring_song_path);

  free(herring_song_path);

}


/* Free shared data: */

void unloadshared(void)
{
  int i;

  for (i = 0; i < 3; i++)
    {
      texture_free(&tux_right[i]);
      texture_free(&tux_left[i]);
      texture_free(&bigtux_right[i]);
      texture_free(&bigtux_left[i]);
    }

  texture_free(&bigtux_right_jump);
  texture_free(&bigtux_left_jump);

  for (i = 0; i < 2; i++)
    {
      texture_free(&cape_right[i]);
      texture_free(&cape_left[i]);
      texture_free(&bigcape_right[i]);
      texture_free(&bigcape_left[i]);
    }

  texture_free(&ducktux_left);
  texture_free(&ducktux_right);

  texture_free(&skidtux_left);
  texture_free(&skidtux_right);

  for (i = 0; i < 4; i++)
    {
      texture_free(&img_bsod_left[i]);
      texture_free(&img_bsod_right[i]);
    }

  texture_free(&img_bsod_squished_left);
  texture_free(&img_bsod_squished_right);

  texture_free(&img_bsod_falling_left);
  texture_free(&img_bsod_falling_right);

  for (i = 0; i < 3; i++)
    {
      texture_free(&img_laptop_left[i]);
      texture_free(&img_laptop_right[i]);
    }

  texture_free(&img_laptop_flat_left);
  texture_free(&img_laptop_flat_right);

  texture_free(&img_laptop_falling_left);
  texture_free(&img_laptop_falling_right);

  for (i = 0; i < 2; i++)
    {
      texture_free(&img_money_left[i]);
      texture_free(&img_money_right[i]);
    }

  texture_free(&img_box_full);
  texture_free(&img_box_empty);

  texture_free(&img_water);
  for (i = 0; i < 3; i++)
    texture_free(&img_waves[i]);

  texture_free(&img_pole);
  texture_free(&img_poletop);

  for (i = 0; i < 2; i++)
    texture_free(&img_flag[i]);

  texture_free(&img_mints);
  texture_free(&img_coffee);

  for (i = 0; i < 4; i++)
    {
      texture_free(&img_distro[i]);
      texture_free(&img_cloud[0][i]);
      texture_free(&img_cloud[1][i]);
    }

  texture_free(&img_golden_herring);

  for (i = 0; i < NUM_SOUNDS; i++)
    free_chunk(sounds[i]);

  /* free the herring song */
  free_music( herring_song );
}


/* Draw a tile on the screen: */

void drawshape(float x, float y, unsigned char c)
{
  int z;

  if (c == 'X' || c == 'x')
    texture_draw(&img_brick[0], x, y, NO_UPDATE);
  else if (c == 'Y' || c == 'y')
    texture_draw(&img_brick[1], x, y, NO_UPDATE);
  else if (c == 'A' || c =='B' || c == '!')
    texture_draw(&img_box_full, x, y, NO_UPDATE);
  else if (c == 'a')
    texture_draw(&img_box_empty, x, y, NO_UPDATE);
  else if (c >= 'C' && c <= 'F')
    texture_draw(&img_cloud[0][c - 'C'], x, y, NO_UPDATE);
  else if (c >= 'c' && c <= 'f')
    texture_draw(&img_cloud[1][c - 'c'], x, y, NO_UPDATE);
  else if (c >= 'G' && c <= 'J')
    texture_draw(&img_bkgd[0][c - 'G'], x, y, NO_UPDATE);
  else if (c >= 'g' && c <= 'j')
    texture_draw(&img_bkgd[1][c - 'g'], x, y, NO_UPDATE);
  else if (c == '#')
    texture_draw(&img_solid[0], x, y, NO_UPDATE);
  else if (c == '[')
    texture_draw(&img_solid[1], x, y, NO_UPDATE);
  else if (c == '=')
    texture_draw(&img_solid[2], x, y, NO_UPDATE);
  else if (c == ']')
    texture_draw(&img_solid[3], x, y, NO_UPDATE);
  else if (c == '$')
    {
      
      z = (frame / 2) % 6;

      if (z < 4)
        texture_draw(&img_distro[z], x, y, NO_UPDATE);
      else if (z == 4)
        texture_draw(&img_distro[2], x, y, NO_UPDATE);
      else if (z == 5)
        texture_draw(&img_distro[1], x, y, NO_UPDATE);
    }
  else if (c == '^')
    {
      z = (frame / 3) % 3;

      texture_draw(&img_waves[z], x, y, NO_UPDATE);
    }
  else if (c == '*')
    texture_draw(&img_poletop, x, y, NO_UPDATE);
  else if (c == '|')
    {
      texture_draw(&img_pole, x, y, NO_UPDATE);

      /* Mark this as the end position of the level! */

      endpos = x;
    }
  else if (c == '\\')
    {
      z = (frame / 3) % 2;

      texture_draw(&img_flag[z], x + 16, y, NO_UPDATE);
    }
  else if (c == '&')
    texture_draw(&img_water, x, y, NO_UPDATE);
}


/* What shape is at some position? */

unsigned char shape(float x, float y)
{

  int xx, yy;
  unsigned char c;

  yy = ((int)y / 32);
  xx = ((int)x / 32);

  if (yy >= 0 && yy < 15 && xx >= 0 && xx <= current_level.width)
  {
    c = current_level.tiles[yy][xx];
    }
  else
    c = '.';
    
  return(c);
}

/* Is is ground? */

int issolid(float x, float y)
{
  if (isbrick(x, y) ||
      isbrick(x + 31, y) ||
      isice(x, y) ||
      isice(x + 31, y) ||
      (shape(x, y) == '[' ||
       shape(x + 31, y) == '[') ||
      (shape(x, y) == '=' ||
       shape(x + 31, y) == '=') ||
      (shape(x, y) == ']' ||
       shape(x + 31, y) == ']') ||
      (shape(x, y) == 'A' ||
       shape(x + 31, y) == 'A') ||
      (shape(x, y) == 'B' ||
       shape(x + 31, y) == 'B') ||
      (shape(x, y) == '!' ||
       shape(x + 31, y) == '!') ||
      (shape(x, y) == 'a' ||
       shape(x + 31, y) == 'a'))
    {
      return YES;
    }

  return NO;
}


/* Is it a brick? */

int isbrick(float x, float y)
{
  if (shape(x, y) == 'X' ||
      shape(x, y) == 'x' ||
      shape(x, y) == 'Y' ||
      shape(x, y) == 'y')
    {
      return YES;
    }

  return NO;
}


/* Is it ice? */

int isice(float x, float y)
{
  if (shape(x, y) == '#')
    {
      return YES;
    }

  return NO;
}


/* Is it a full box? */

int isfullbox(float x, float y)
{
  if (shape(x, y) == 'A' ||
      shape(x, y) == 'B' ||
      shape(x, y) == '!')
    {
      return YES;
    }

  return NO;
}

/* Break a brick: */

void trybreakbrick(float x, float y)
{
  if (isbrick(x, y))
    {
      if (shape(x, y) == 'x' || shape(x, y) == 'y')
        {
          /* Get a distro from it: */

          add_bouncy_distro(((x + 1) / 32) * 32,
                            (int)(y / 32) * 32);

          if (counting_distros == NO)
            {
              counting_distros = YES;
              distro_counter = 50;
            }

          if (distro_counter <= 0)
            level_change(&current_level,x, y, 'a');

          play_sound(sounds[SND_DISTRO], SOUND_CENTER_SPEAKER);
          score = score + SCORE_DISTRO;
          distros++;
        }
      else
        {
          /* Get rid of it: */

          level_change(&current_level,x, y,'.');
        }


      /* Replace it with broken bits: */

      add_broken_brick(((x + 1) / 32) * 32,
                       (int)(y / 32) * 32);


      /* Get some score: */

      play_sound(sounds[SND_BRICK], SOUND_CENTER_SPEAKER);
      score = score + SCORE_BRICK;
    }
}


/* Bounce a brick: */

void bumpbrick(float x, float y)
{
  add_bouncy_brick(((int)(x + 1) / 32) * 32,
                   (int)(y / 32) * 32);

  play_sound(sounds[SND_BRICK], SOUND_CENTER_SPEAKER);
}


/* Empty a box: */

void tryemptybox(float x, float y)
{
  if (isfullbox(x, y))
    {
      if (shape(x, y) == 'A')
        {
          /* Box with a distro! */

          add_bouncy_distro(((x + 1) / 32) * 32,
                            (int)(y / 32) * 32 - 32);

          play_sound(sounds[SND_DISTRO], SOUND_CENTER_SPEAKER);
          score = score + SCORE_DISTRO;
          distros++;
        }
      else if (shape(x, y) == 'B')
        {
          /* Add an upgrade! */

          if (tux.size == SMALL)
            {
              /* Tux is small, add mints! */

              add_upgrade(((x + 1) / 32) * 32,
                          (int)(y / 32) * 32 - 32,
                          UPGRADE_MINTS);
            }
          else
            {
              /* Tux is big, add coffee: */

              add_upgrade(((x + 1) / 32) * 32,
                          (int)(y / 32) * 32 - 32,
                          UPGRADE_COFFEE);
            }

          play_sound(sounds[SND_UPGRADE], SOUND_CENTER_SPEAKER);
        }
      else if (shape(x, y) == '!')
        {
          /* Add a golden herring */

          add_upgrade(((x + 1) / 32) * 32,
                      (int)(y / 32) * 32 - 32,
                      UPGRADE_HERRING);
        }

      /* Empty the box: */

      level_change(&current_level,x, y, 'a');
    }
}


/* Try to grab a distro: */

void trygrabdistro(float x, float y, int bounciness)
{
  if (shape(x, y) == '$')
    {
      level_change(&current_level,x, y, '.');
      play_sound(sounds[SND_DISTRO], SOUND_CENTER_SPEAKER);

      if (bounciness == BOUNCE)
        {
          add_bouncy_distro(((x + 1) / 32) * 32,
                            (int)(y / 32) * 32);
        }

      score = score + SCORE_DISTRO;
      distros++;
    }
}

/* Try to bump a bad guy from below: */

void trybumpbadguy(float x, float y)
{
  int i;


  /* Bad guys: */

  for (i = 0; i < NUM_BAD_GUYS; i++)
    {
      if (bad_guys[i].alive &&
          bad_guys[i].x >= x - 32 && bad_guys[i].x <= x + 32 &&
          bad_guys[i].y >= y - 16 && bad_guys[i].y <= y + 16)
        {
          if (bad_guys[i].kind == BAD_BSOD ||
              bad_guys[i].kind == BAD_LAPTOP)
            {
              bad_guys[i].dying = FALLING;
              bad_guys[i].ym = -8;
              play_sound(sounds[SND_FALL], SOUND_CENTER_SPEAKER);
            }
        }
    }


  /* Upgrades: */

  for (i = 0; i < NUM_UPGRADES; i++)
    {
      if (upgrades[i].alive && upgrades[i].height == 32 &&
          upgrades[i].x >= x - 32 && upgrades[i].x <= x + 32 &&
          upgrades[i].y >= y - 16 && upgrades[i].y <= y + 16)
        {
          upgrades[i].xm = -upgrades[i].xm;
          upgrades[i].ym = -8;
          play_sound(sounds[SND_BUMP_UPGRADE], SOUND_CENTER_SPEAKER);
        }
    }
}


/* Add an upgrade: */

void add_upgrade(float x, float y, int kind)
{
  int i, found;

  found = -1;

  for (i = 0; i < NUM_UPGRADES && found == -1; i++)
    {
      if (!upgrades[i].alive)
        found = i;
    }

  if (found != -1)
    {
      upgrades[found].alive = YES;
      upgrades[found].kind = kind;
      upgrades[found].x = x;
      upgrades[found].y = y;
      upgrades[found].xm = 2;
      upgrades[found].ym = -2;
      upgrades[found].height = 0;
    }
}

/* Add a bullet: */

void add_bullet(float x, float y, float xm, int dir)
{
  int i, found;
  
  printf("X: %f Y: %f -- YOOYOYOYO\n",x,y);

  found = -1;

  for (i = 0; i < NUM_BULLETS && found == -1; i++)
    {
      if (!bullets[i].alive)
        found = i;
    }

  if (found != -1)
    {
      bullets[found].alive = YES;

      if (dir == RIGHT)
        {
          bullets[found].x = x + 32;
          bullets[found].xm = BULLET_XM + xm;
        }
      else
        {
          bullets[found].x = x;
          bullets[found].xm = -BULLET_XM + xm;
        }

      bullets[found].y = y;
      bullets[found].ym = BULLET_STARTING_YM;

      play_sound(sounds[SND_SHOOT], SOUND_CENTER_SPEAKER);
    }
}


/* (Status): */
void drawstatus(void)
{
int i;

  sprintf(str, "%d", score);
  drawtext("SCORE", 0, 0, letters_blue, NO_UPDATE, 1);
  drawtext(str, 96, 0, letters_gold, NO_UPDATE, 1);

  sprintf(str, "%d", highscore);
  drawtext("HIGH", 0, 20, letters_blue, NO_UPDATE, 1);
  drawtext(str, 96, 20, letters_gold, NO_UPDATE, 1);

  if (timer_get_left(&time_left) > TIME_WARNING || (frame % 10) < 5)
    {
      sprintf(str, "%d", timer_get_left(&time_left) / 1000 );
      drawtext("TIME", 224, 0, letters_blue, NO_UPDATE, 1);
      drawtext(str, 304, 0, letters_gold, NO_UPDATE, 1);
    }

  sprintf(str, "%d", distros);
  drawtext("DISTROS", screen->h, 0, letters_blue, NO_UPDATE, 1);
  drawtext(str, 608, 0, letters_gold, NO_UPDATE, 1);

  drawtext("LIVES", screen->h, 20, letters_blue, NO_UPDATE, 1);

  for(i=0; i < tux.lives; ++i)
    {
      texture_draw(&tux_life,565+(18*i),20,NO_UPDATE);
    }
}


void drawendscreen(void)
{
  char str[80];

  clearscreen(0, 0, 0);

  drawcenteredtext("GAMEOVER", 200, letters_red, NO_UPDATE, 1);

  sprintf(str, "SCORE: %d", score);
  drawcenteredtext(str, 224, letters_gold, NO_UPDATE, 1);

  sprintf(str, "DISTROS: %d", distros);
  drawcenteredtext(str, 256, letters_blue, NO_UPDATE, 1);

  flipscreen();
  SDL_Delay(2000);
}

void drawresultscreen(void)
{
  char str[80];

  clearscreen(0, 0, 0);

  drawcenteredtext("Result:", 200, letters_red, NO_UPDATE, 1);

  sprintf(str, "SCORE: %d", score);
  drawcenteredtext(str, 224, letters_gold, NO_UPDATE, 1);

  sprintf(str, "DISTROS: %d", distros);
  drawcenteredtext(str, 256, letters_blue, NO_UPDATE, 1);

  flipscreen();
  SDL_Delay(2000);
}

void savegame(void)
{
  char savefile[300];
  time_t current_time = time(NULL);
  struct tm* time_struct;
  FILE* fi;

  time_struct = localtime(&current_time);
  sprintf(savefile,"%s/%d-%d-%d-%d.save",st_save_dir,time_struct->tm_year+1900,time_struct->tm_mon,time_struct->tm_mday,time_struct->tm_hour);
  printf("%s",savefile);


  fi = fopen(savefile, "wb");

  if (fi == NULL)
    {
      fprintf(stderr, "Warning: I could not open the high score file ");

    }
  else
    {
      fwrite(&level,4,1,fi);
      fwrite(&score,4,1,fi);
      fwrite(&distros,4,1,fi);
      fwrite(&tux.x,4,1,fi);
      fwrite(&tux.y,4,1,fi);
      fwrite(&scroll_x,4,1,fi);
      fwrite(&current_level.time_left,4,1,fi);
    }
  fclose(fi);

}

void loadgame(char* filename)
{
  char savefile[300];
  FILE* fi;
  time_t current_time = time(NULL);
  struct tm* time_struct;

  time_struct = localtime(&current_time);
  sprintf(savefile,"%s/%d-%d-%d-%d.save",st_save_dir,time_struct->tm_year+1900,time_struct->tm_mon,time_struct->tm_mday,time_struct->tm_hour);
  printf("%s",savefile);


  fi = fopen(savefile, "rb");

  if (fi == NULL)
    {
      fprintf(stderr, "Warning: I could not open the high score file ");

    }
  else
    {
    player_level_begin(&tux);
      set_defaults();
      loadlevel(&current_level,"default",level);
      activate_bad_guys();
      unloadlevelgfx();
      loadlevelgfx(&current_level);
      unloadlevelsong();
      loadlevelsong();
      levelintro();
      start_timers();

      fread(&level,4,1,fi);
      fread(&score,4,1,fi);
      fread(&distros,4,1,fi);
      fread(&tux.x,4,1,fi);
      fread(&tux.y,4,1,fi);
      fread(&scroll_x,4,1,fi);
      fread(&current_level.time_left,4,1,fi);
      fclose(fi);
    }

}
