/*
  gameloop.c
  
  Super Tux - Game Loop!
  
  by Bill Kendrick & Tobias Glaesser
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/supertux/
  
  April 11, 2000 - March 15, 2004
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

st_level current_level;
int game_started = NO;

/* Local variables: */

static texture_type img_waves[3], img_water, img_pole, img_poletop, img_flag[2];
static texture_type img_cloud[2][4];
static SDL_Event event;
static SDLKey key;
static char level_subset[100];
static char str[60];
static float fps_fps;
static int st_gl_mode;
static unsigned int last_update_time;
static unsigned int update_time;
static int pause_menu_frame;
static int debug_fps;

/* Local function prototypes: */

void levelintro(void);
void loadshared(void);
void unloadshared(void);
void drawstatus(void);
void drawendscreen(void);
void drawresultscreen(void);

#define JOYSTICK_DEAD_ZONE 4096

void levelintro(void)
{
  /* Level Intro: */

  clearscreen(0, 0, 0);

  sprintf(str, "LEVEL %d", level);
  text_drawf(&blue_text, str, 0, 200, A_HMIDDLE, A_TOP, 1, NO_UPDATE);

  sprintf(str, "%s", current_level.name);
  text_drawf(&gold_text, str, 0, 224, A_HMIDDLE, A_TOP, 1, NO_UPDATE);

  sprintf(str, "TUX x %d", tux.lives);
  text_drawf(&white_text, str, 0, 256, A_HMIDDLE, A_TOP, 1, NO_UPDATE);

  flipscreen();

  SDL_Delay(1000);
}

/* Reset Timers */
void start_timers(void)
{
  timer_start(&time_left,current_level.time_left*1000);
  st_pause_ticks_init();
  update_time = st_get_ticks();
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
            menu_event(&event.key.keysym);

          if(player_key_event(&tux,key,DOWN))
            break;

          switch(key)
            {
            case SDLK_ESCAPE:    /* Escape: Open/Close the menu: */
              if(!game_pause)
                {
                  if(st_gl_mode == ST_GL_TEST)
                    quit = 1;
                  else if(show_menu)
                    {
                      menu_set_current(&game_menu);
                      show_menu = 0;
                      st_pause_ticks_stop();
                    }
                  else
                    {
                      menu_set_current(&game_menu);
                      show_menu = 1;
                      st_pause_ticks_start();
                    }
                }
              break;
            default:
              break;
            }
          break;
        case SDL_KEYUP:      /* A keyrelease! */
          key = event.key.keysym.sym;

          if(player_key_event(&tux,key,UP))
            break;

          switch(key)
            {
            case SDLK_p:
              if(!show_menu)
                {
                  if(game_pause)
                    {
                      game_pause = 0;
                      st_pause_ticks_stop();
                    }
                  else
                    {
                      game_pause = 1;
                      st_pause_ticks_start();
                    }
                }
              break;
            case SDLK_TAB:
              if(debug_mode == YES)
                {
                  tux.size = !tux.size;
                  if(tux.size == BIG)
                    {
                      tux.base.height = 64;
                    }
                  else
                    tux.base.height = 32;
                }
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
            case SDLK_l:
              if(debug_mode == YES)
                --tux.lives;
              break;
            case SDLK_s:
              if(debug_mode == YES)
                score += 1000;
            case SDLK_f:
              if(debug_fps == YES)
	      debug_fps = NO;
	      else
	      debug_fps = YES;
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
              if (event.jaxis.value < -JOYSTICK_DEAD_ZONE)
                tux.input.left = DOWN;
              else if (event.jaxis.value > JOYSTICK_DEAD_ZONE)
                tux.input.left = UP;

              if (event.jaxis.value > JOYSTICK_DEAD_ZONE)
                tux.input.right = DOWN;
              else if (event.jaxis.value < -JOYSTICK_DEAD_ZONE)
                tux.input.right = UP;
              break;
            case JOY_Y:
              if (event.jaxis.value > JOYSTICK_DEAD_ZONE)
                tux.input.down = DOWN;
              else if (event.jaxis.value < -JOYSTICK_DEAD_ZONE)
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

#endif
        default:
          break;

        }  /* switch */

    } /* while */

}

/* --- GAME ACTION! --- */

int game_action(void)
{
  int i;

  /* (tux_dying || next_level) */
  if (tux.dying || next_level)
    {
      /* Tux either died, or reached the end of a level! */

      halt_music();


      if (next_level)
        {
          /* End of a level! */
          level++;
          next_level = 0;
          if(st_gl_mode != ST_GL_TEST)
            {
              drawresultscreen();
            }
          else
            {
              level_free_gfx();
              level_free(&current_level);
              level_free_song();
              unloadshared();
              arrays_free();
              return(0);
            }
          player_level_begin(&tux);
        }
      else
        {
          player_dying(&tux);

          /* No more lives!? */

          if (tux.lives < 0)
            {
              if(st_gl_mode != ST_GL_TEST)
                drawendscreen();

              if(st_gl_mode != ST_GL_TEST)
                {
                  if (score > hs_score)
                    save_hs(score);
                }
              level_free_gfx();
              level_free(&current_level);
              level_free_song();
              unloadshared();
              arrays_free();
              return(0);
            } /* if (lives < 0) */
        }

      /* Either way, (re-)load the (next) level... */

      player_level_begin(&tux);
      set_defaults();
      level_free(&current_level);
      if(level_load(&current_level,level_subset,level) != 0)
        return 0;
      arrays_free();
      arrays_init();
      activate_bad_guys();
      level_free_gfx();
      level_load_gfx(&current_level);
      level_free_song();
      level_load_song(&current_level);
      if(st_gl_mode != ST_GL_TEST)
        levelintro();
      start_timers();
      /* Play music: */
      play_current_music();
    }

  player_action(&tux);

  /* Handle bouncy distros: */

  for (i = 0; i < num_bouncy_distros; i++)
    {
      bouncy_distro_action(&bouncy_distros[i]);
    }


  /* Handle broken bricks: */

  for (i = 0; i < num_broken_bricks; i++)
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

  for (i = 0; i < num_bouncy_bricks; i++)
    {
      bouncy_brick_action(&bouncy_bricks[i]);
    }


  /* Handle floating scores: */

  for (i = 0; i < num_floating_scores; i++)
    {
      floating_score_action(&floating_scores[i]);
    }


  /* Handle bullets: */

  for (i = 0; i < num_bullets; ++i)
    {
      bullet_action(&bullets[i]);
    }

  /* Handle upgrades: */

  for (i = 0; i < num_upgrades; i++)
    {
      upgrade_action(&upgrades[i]);
    }


  /* Handle bad guys: */

  for (i = 0; i < num_bad_guys; i++)
    {
      badguy_action(&bad_guys[i]);
    }

  /* Handle all possible collisions. */
  collision_handler();

  return -1;
}

/* --- GAME DRAW! --- */

void game_draw(void)
{
  int  x, y, i, s;

  /* Draw screen: */

  if (tux.dying && (frame % 4) == 0)
    clearscreen(255, 255, 255);
  else if(timer_check(&super_bkgd_timer))
    texture_draw(&img_super_bkgd, 0, 0, NO_UPDATE);
  else
    {
      /* Draw the real background */
      if(current_level.bkgd_image[0] != '\0')
        {
          s = (int)scroll_x / 30;
          texture_draw_part(&img_bkgd,s,0,0,0,img_bkgd.w - s, img_bkgd.h, NO_UPDATE);
          texture_draw_part(&img_bkgd,0,0,screen->w - s ,0,s,img_bkgd.h, NO_UPDATE);
        }
      else
        {
          clearscreen(current_level.bkgd_red, current_level.bkgd_green, current_level.bkgd_blue);
        }
    }

  /* Draw background: */

  for (y = 0; y < 15; ++y)
    {
      for (x = 0; x < 21; ++x)
        {
          drawshape(x * 32 - ((int)scroll_x % 32), y * 32,
                    current_level.tiles[(int)y][(int)x + (int)(scroll_x / 32)]);
        }
    }


  /* (Bouncy bricks): */

  for (i = 0; i < num_bouncy_bricks; ++i)
    {
      bouncy_brick_draw(&bouncy_bricks[i]);
    }


  /* (Bad guys): */

  for (i = 0; i < num_bad_guys; ++i)
    {
      badguy_draw(&bad_guys[i]);
    }

  /* (Tux): */

  player_draw(&tux);

  /* (Bullets): */

  for (i = 0; i < num_bullets; ++i)
    {
      bullet_draw(&bullets[i]);
    }

  /* (Floating scores): */

  for (i = 0; i < num_floating_scores; ++i)
    {
      floating_score_draw(&floating_scores[i]);
    }


  /* (Upgrades): */

  for (i = 0; i < num_upgrades; ++i)
    {
      upgrade_draw(&upgrades[i]);
    }


  /* (Bouncy distros): */

  for (i = 0; i < num_bouncy_distros; ++i)
    {
      bouncy_distro_draw(&bouncy_distros[i]);
    }


  /* (Broken bricks): */

  for (i = 0; i < num_broken_bricks; ++i)
    {
      broken_brick_draw(&broken_bricks[i]);
    }

  drawstatus();


  if(game_pause)
    {
      x = screen->h / 20;
      for(i = 0; i < x; ++i)
        {
          fillrect(i % 2 ? (pause_menu_frame * i)%screen->w : -((pause_menu_frame * i)%screen->w) ,(i*20+pause_menu_frame)%screen->h,screen->w,10,20,20,20, rand() % 20 + 1);
        }
      fillrect(0,0,screen->w,screen->h,rand() % 50, rand() % 50, rand() % 50, 128);
      text_drawf(&blue_text, "PAUSE - Press 'P' To Play", 0, 230, A_HMIDDLE, A_TOP, 1, NO_UPDATE);
    }

  if(show_menu)
    menu_process_current();

  /* (Update it all!) */

  updatescreen();


}

/* --- GAME LOOP! --- */

int gameloop(char * subset, int levelnb, int mode)
{
  int fps_cnt, jump, done;
  timer_type fps_timer, frame_timer;
  timer_init(&fps_timer, YES);
  timer_init(&frame_timer, YES);

  game_started = YES;

  st_gl_mode = mode;
  level = levelnb;
  strcpy(level_subset,subset);

  /* Init the game: */
  arrays_init();
  set_defaults();

  if(level_load(&current_level,level_subset,level) != 0)
    exit(1);
  level_load_gfx(&current_level);
  activate_bad_guys();
  level_load_song(&current_level);

  player_init(&tux);

  if(st_gl_mode != ST_GL_TEST)
    load_hs();

  loadshared();

  if(st_gl_mode == ST_GL_PLAY)
    levelintro();


  timer_init(&time_left,YES);
  start_timers();

  if(st_gl_mode == ST_GL_LOAD_GAME)
    loadgame(levelnb);


  /* --- MAIN GAME LOOP!!! --- */

  jump = NO;
  done = 0;
  quit = 0;
  frame = 0;
  game_pause = 0;
  timer_init(&fps_timer,YES);
  timer_init(&frame_timer,YES);
  fps_cnt = 0;

  /* Clear screen: */

  clearscreen(0, 0, 0);
  updatescreen();

  /* Play music: */
  play_current_music();


  while (SDL_PollEvent(&event))
  {}

  game_draw();
  do
    {
      jump = NO;

      /* Calculate the movement-factor */
      frame_ratio = ((double)(update_time-last_update_time))/((double)FRAME_RATE);
      if(frame_ratio > 1.5) /* Quick hack to correct the unprecise CPU clocks a little bit. */
      frame_ratio = 1.5 + (frame_ratio - 1.5) * 0.85;
      
      if(!timer_check(&frame_timer))
        {
          timer_start(&frame_timer,25);
          ++frame;
        }

      /* Handle events: */

      tux.input.old_fire = tux.input.fire;

      game_event();

      if(show_menu)
        {
          if(current_menu == &game_menu)
            {
              switch (menu_check(&game_menu))
                {
                case 2:
                  st_pause_ticks_stop();
                  break;
                case 3:
                  update_load_save_game_menu(&save_game_menu, NO);
                  break;
                case 4:
                  update_load_save_game_menu(&load_game_menu, YES);
                  break;
                case 7:
                  st_pause_ticks_stop();
                  done = 1;
                  break;
                }
            }
          else if(current_menu == &options_menu)
            {
              process_options_menu();
            }
          else if(current_menu == &save_game_menu )
            {
              process_save_load_game_menu(YES);
            }
          else if(current_menu == &load_game_menu )
            {
              process_save_load_game_menu(NO);
            }
        }


      /* Handle actions: */

      if(!game_pause && !show_menu)
        {
	/*float z = frame_ratio;
	frame_ratio = 1;
	while(z >= 1)
	{*/
          if (game_action() == 0)
            {
              /* == 0: no more lives */
              /* == -1: continues */
              return 0;
            }
	  /*  --z;
	    }*/
        }
      else
        {
          ++pause_menu_frame;
          SDL_Delay(50);
        }

      if(debug_mode && debug_fps == YES)
        SDL_Delay(60);

      /*Draw the current scene to the screen */
      /*If the machine running the game is too slow
        skip the drawing of the frame (so the calculations are more precise and
      the FPS aren't affected).*/
      /*if( ! fps_fps < 50.0 )
      game_draw();
      else
      jump = YES;*/ /*FIXME: Implement this tweak right.*/
      game_draw();

      /* Time stops in pause mode */
      if(game_pause || show_menu )
        {
          continue;
        }

      /* Set the time of the last update and the time of the current update */
      last_update_time = update_time;
      update_time = st_get_ticks();


      /* Pause till next frame, if the machine running the game is too fast: */
      /* FIXME: Works great for in OpenGl mode, where the CPU doesn't have to do that much. But
                the results in SDL mode aren't perfect (thought the 100 FPS are reached), even on an AMD2500+. */
      if(last_update_time >= update_time - 12 && jump != YES )
        SDL_Delay(10);
      /*if((update_time - last_update_time) < 10)
          SDL_Delay((11 - (update_time - last_update_time))/2);*/



      /* Handle time: */

      if (timer_check(&time_left))
        {
          /* are we low on time ? */
          if ((timer_get_left(&time_left) < TIME_WARNING)
              && (get_current_music() != HURRYUP_MUSIC))     /* play the fast music */
             {
              set_current_music(HURRYUP_MUSIC);
              play_current_music();
             }

        }
      else
        player_kill(&tux,KILL);


      /* Calculate frames per second */
      if(show_fps)
        {
          ++fps_cnt;
          fps_fps = (1000.0 / (float)timer_get_gone(&fps_timer)) * (float)fps_cnt;

          if(!timer_check(&fps_timer))
            {
              timer_start(&fps_timer,1000);
              fps_cnt = 0;
            }
        }

    }
  while (!done && !quit);

  halt_music();

  level_free_gfx();
  level_free(&current_level);
  level_free_song();
  unloadshared();
  arrays_free();

  game_started = NO;

  return(quit);
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
                                      strlen("SALCON.MOD") + 8));

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
    texture_draw(&img_bkgd_tile[0][c - 'G'], x, y, NO_UPDATE);
  else if (c >= 'g' && c <= 'j')
    texture_draw(&img_bkgd_tile[1][c - 'g'], x, y, NO_UPDATE);
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
      isice(x, y) ||
      (shape(x, y) == '[') ||
      (shape(x, y) == '=') ||
      (shape(x, y) == ']') ||
      (shape(x, y) == 'A') ||
      (shape(x, y) == 'B') ||
      (shape(x, y) == '!') ||
      (shape(x, y) == 'a'))
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

          add_bouncy_distro(((int)(x + 1) / 32) * 32,
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

      add_broken_brick(((int)(x + 1) / 32) * 32,
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

void tryemptybox(float x, float y, int col_side)
{
if (!isfullbox(x, y))
  return;

// according to the collision side, set the upgrade direction

if(col_side == LEFT)
  col_side = RIGHT;
else
  col_side = LEFT;

switch(shape(x,y))
  {
  case 'A':      /* Box with a distro! */
    add_bouncy_distro(((int)(x + 1) / 32) * 32, (int)(y / 32) * 32 - 32);
    play_sound(sounds[SND_DISTRO], SOUND_CENTER_SPEAKER);
    score = score + SCORE_DISTRO;
    distros++;
    break;
  case 'B':      /* Add an upgrade! */
    if (tux.size == SMALL)     /* Tux is small, add mints! */
      add_upgrade((int)((x + 1) / 32) * 32, (int)(y / 32) * 32 - 32, col_side, UPGRADE_MINTS);
    else     /* Tux is big, add coffee: */
      add_upgrade((int)((x + 1) / 32) * 32, (int)(y / 32) * 32 - 32, col_side, UPGRADE_COFFEE);
    play_sound(sounds[SND_UPGRADE], SOUND_CENTER_SPEAKER);
    break;
  case '!':     /* Add a golden herring */
    add_upgrade((int)((x + 1) / 32) * 32, (int)(y / 32) * 32 - 32, col_side, UPGRADE_HERRING);
    break;
  default:
    break;
  }

/* Empty the box: */
level_change(&current_level,x, y, 'a');
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
          add_bouncy_distro(((int)(x + 1) / 32) * 32,
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

  for (i = 0; i < num_bad_guys; i++)
    {
      if (bad_guys[i].base.alive &&
          bad_guys[i].base.x >= x - 32 && bad_guys[i].base.x <= x + 32 &&
          bad_guys[i].base.y >= y - 16 && bad_guys[i].base.y <= y + 16)
        {
          if (bad_guys[i].kind == BAD_BSOD ||
              bad_guys[i].kind == BAD_LAPTOP)
            {
              bad_guys[i].dying = FALLING;
              bad_guys[i].base.ym = -8;
              play_sound(sounds[SND_FALL], SOUND_CENTER_SPEAKER);
            }
        }
    }


  /* Upgrades: */

  for (i = 0; i < num_upgrades; i++)
    {
      if (upgrades[i].base.alive && upgrades[i].base.height == 32 &&
          upgrades[i].base.x >= x - 32 && upgrades[i].base.x <= x + 32 &&
          upgrades[i].base.y >= y - 16 && upgrades[i].base.y <= y + 16)
        {
          upgrades[i].base.xm = -upgrades[i].base.xm;
          upgrades[i].base.ym = -8;
          play_sound(sounds[SND_BUMP_UPGRADE], SOUND_CENTER_SPEAKER);
        }
    }
}

/* (Status): */
void drawstatus(void)
{
  int i;

  sprintf(str, "%d", score);
  text_draw(&white_text, "SCORE", 0, 0, 1, NO_UPDATE);
  text_draw(&gold_text, str, 96, 0, 1, NO_UPDATE);

  if(st_gl_mode != ST_GL_TEST)
    {
      sprintf(str, "%d", hs_score);
      text_draw(&white_text, "HIGH", 0, 20, 1, NO_UPDATE);
      text_draw(&gold_text, str, 96, 20, 1, NO_UPDATE);
    }
  else
    {
      text_draw(&white_text,"Press ESC To Return",0,20,1, NO_UPDATE);
    }

  if (timer_get_left(&time_left) > TIME_WARNING || (frame % 10) < 5)
    {
      sprintf(str, "%d", timer_get_left(&time_left) / 1000 );
      text_draw(&white_text, "TIME", 224, 0, 1, NO_UPDATE);
      text_draw(&gold_text, str, 304, 0, 1, NO_UPDATE);
    }

  sprintf(str, "%d", distros);
  text_draw(&white_text, "DISTROS", screen->h, 0, 1, NO_UPDATE);
  text_draw(&gold_text, str, 608, 0, 1, NO_UPDATE);

  text_draw(&white_text, "LIVES", screen->h, 20, 1, NO_UPDATE);

  if(show_fps)
    {
      sprintf(str, "%2.1f", fps_fps);
      text_draw(&white_text, "FPS", screen->h, 40, 1, NO_UPDATE);
      text_draw(&gold_text, str, screen->h + 60, 40, 1, NO_UPDATE);
    }

  for(i=0; i < tux.lives; ++i)
    {
      texture_draw(&tux_life,565+(18*i),20,NO_UPDATE);
    }
}


void drawendscreen(void)
{
  char str[80];

  clearscreen(0, 0, 0);

  text_drawf(&blue_text, "GAMEOVER", 0, 200, A_HMIDDLE, A_TOP, 1, NO_UPDATE);

  sprintf(str, "SCORE: %d", score);
  text_drawf(&gold_text, str, 0, 224, A_HMIDDLE, A_TOP, 1, NO_UPDATE);

  sprintf(str, "DISTROS: %d", distros);
  text_drawf(&gold_text, str, 0, 256, A_HMIDDLE, A_TOP, 1, NO_UPDATE);

  flipscreen();
  SDL_Delay(2000);
}

void drawresultscreen(void)
{
  char str[80];

  clearscreen(0, 0, 0);

  text_drawf(&blue_text, "Result:", 0, 200, A_HMIDDLE, A_TOP, 1, NO_UPDATE);

  sprintf(str, "SCORE: %d", score);
  text_drawf(&gold_text, str, 0, 224, A_HMIDDLE, A_TOP, 1, NO_UPDATE);

  sprintf(str, "DISTROS: %d", distros);
  text_drawf(&gold_text, str, 0, 256, A_HMIDDLE, A_TOP, 1, NO_UPDATE);

  flipscreen();
  SDL_Delay(2000);
}

void savegame(int slot)
{
  char savefile[1024];
  FILE* fi;
  unsigned int ui;

  sprintf(savefile,"%s/slot%d.save",st_save_dir,slot);

  fi = fopen(savefile, "wb");

  if (fi == NULL)
    {
      fprintf(stderr, "Warning: I could not open the slot file ");

    }
  else
    {
      fputs(level_subset, fi);
      fputs("\n", fi);
      fwrite(&level,sizeof(int),1,fi);
      fwrite(&score,sizeof(int),1,fi);
      fwrite(&distros,sizeof(int),1,fi);
      fwrite(&scroll_x,sizeof(float),1,fi);
      fwrite(&tux,sizeof(player_type),1,fi);
      timer_fwrite(&tux.invincible_timer,fi);
      timer_fwrite(&tux.skidding_timer,fi);
      timer_fwrite(&tux.safe_timer,fi);
      timer_fwrite(&tux.frame_timer,fi);
      timer_fwrite(&time_left,fi);
      ui = st_get_ticks();
      fwrite(&ui,sizeof(int),1,fi);
    }
  fclose(fi);

}

void loadgame(int slot)
{
  char savefile[1024];
  char str[100];
  FILE* fi;
  unsigned int ui;

  sprintf(savefile,"%s/slot%d.save",st_save_dir,slot);

  fi = fopen(savefile, "rb");

  if (fi == NULL)
    {
      fprintf(stderr, "Warning: I could not open the slot file ");

    }
  else
    {


      fgets(str, 100, fi);
      strcpy(level_subset, str);
      level_subset[strlen(level_subset)-1] = '\0';
      fread(&level,sizeof(int),1,fi);

      set_defaults();
      level_free(&current_level);
      if(level_load(&current_level,level_subset,level) != 0)
        exit(1);
      arrays_free();
      arrays_init();
      activate_bad_guys();
      level_free_gfx();
      level_load_gfx(&current_level);
      level_free_song();
      level_load_song(&current_level);
      levelintro();
      update_time = st_get_ticks();

      fread(&score,sizeof(int),1,fi);
      fread(&distros,sizeof(int),1,fi);
      fread(&scroll_x,sizeof(float),1,fi);
      fread(&tux,sizeof(player_type),1,fi);
      timer_fread(&tux.invincible_timer,fi);
      timer_fread(&tux.skidding_timer,fi);
      timer_fread(&tux.safe_timer,fi);
      timer_fread(&tux.frame_timer,fi);
      timer_fread(&time_left,fi);
      fread(&ui,sizeof(int),1,fi);
      tux.hphysic.start_time += st_get_ticks() - ui;
      tux.vphysic.start_time += st_get_ticks() - ui;
      fclose(fi);
    }

}

void slotinfo(char **pinfo, int slot)
{
  FILE* fi;
  char slotfile[1024];
  char tmp[200];
  char str[5];
  int slot_level;
  sprintf(slotfile,"%s/slot%d.save",st_save_dir,slot);

  fi = fopen(slotfile, "rb");

  sprintf(tmp,"Slot %d - ",slot);

  if (fi == NULL)
    {
      strcat(tmp,"Free");
    }
  else
    {
      fgets(str, 100, fi);
      str[strlen(str)-1] = '\0';
      strcat(tmp, str);
      strcat(tmp, " / Level:");
      fread(&slot_level,sizeof(int),1,fi);
      sprintf(str,"%d",slot_level);
      strcat(tmp,str);
      fclose(fi);
    }

  *pinfo = (char*) malloc(sizeof(char) * (strlen(tmp)+1));
  strcpy(*pinfo,tmp);

}

