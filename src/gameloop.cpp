/*
  gameloop.c
  
  Super Tux - Game Loop!
  
  by Bill Kendrick & Tobias Glaesser
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/supertux/
  
  April 11, 2000 - March 15, 2004
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <SDL.h>

#ifndef WIN32
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
#include "tile.h"
#include "particlesystem.h"

/* extern variables */

int game_started = false;

/* Local variables: */
static SDL_Event event;
static SDLKey key;
static char level_subset[100];
static float fps_fps;
static int st_gl_mode;
static unsigned int last_update_time;
static unsigned int update_time;
static int pause_menu_frame;
static int debug_fps;

GameSession* GameSession::current_ = 0;

/* Local function prototypes: */
void levelintro(void);
void loadshared(void);
void unloadshared(void);
void drawstatus(void);
void drawendscreen(void);
void drawresultscreen(void);

GameSession::GameSession()
{
  current_ = this;
  assert(0);
}

GameSession::GameSession(const std::string& filename)
{
  current_ = this;

  world = &::world;

  timer_init(&fps_timer, true);
  timer_init(&frame_timer, true);

  world->load(filename);
}

GameSession::GameSession(const std::string& subset, int levelnb, int mode)
{
  current_ = this;

  world = &::world;

  timer_init(&fps_timer, true);
  timer_init(&frame_timer, true);

  game_started = true;

  st_gl_mode = mode;
  level = levelnb;

  /* Init the game: */
  world->arrays_free();
  set_defaults();

  strcpy(level_subset, subset.c_str());

  if (st_gl_mode == ST_GL_LOAD_LEVEL_FILE)
    {
      if (world->load(level_subset))
        exit(1);
    }
  else
    {
      if(world->load(level_subset, level) != 0)
        exit(1);
    }

  world->get_level()->load_gfx();
  loadshared();
  activate_bad_guys(world->get_level());
  world->activate_particle_systems();
  world->get_level()->load_song();

  tux.init();

  if(st_gl_mode != ST_GL_TEST)
    load_hs();

  if(st_gl_mode == ST_GL_PLAY || st_gl_mode == ST_GL_LOAD_LEVEL_FILE)
    levelintro();

  timer_init(&time_left,true);
  start_timers();

  if(st_gl_mode == ST_GL_LOAD_GAME)
    loadgame(levelnb);
}

void
GameSession::levelintro(void)
{
  char str[60];
  /* Level Intro: */
  clearscreen(0, 0, 0);

  sprintf(str, "LEVEL %d", level);
  text_drawf(&blue_text, str, 0, 200, A_HMIDDLE, A_TOP, 1);

  sprintf(str, "%s", world->get_level()->name.c_str());
  text_drawf(&gold_text, str, 0, 224, A_HMIDDLE, A_TOP, 1);

  sprintf(str, "TUX x %d", tux.lives);
  text_drawf(&white_text, str, 0, 256, A_HMIDDLE, A_TOP, 1);

  flipscreen();

  SDL_Event event;
  wait_for_event(event,1000,3000,true);
}

/* Reset Timers */
void
GameSession::start_timers()
{
  timer_start(&time_left, world->get_level()->time_left*1000);
  st_pause_ticks_init();
  update_time = st_get_ticks();
}

void activate_bad_guys(Level* plevel)
{
  for (std::vector<BadGuyData>::iterator i = plevel->badguy_data.begin();
       i != plevel->badguy_data.end();
       ++i)
    {
      world.add_bad_guy(i->x, i->y, i->kind);
    }
}

void
GameSession::process_events()
{
  while (SDL_PollEvent(&event))
    {
      /* Check for menu-events, if the menu is shown */
      if(show_menu)
        menu_event(event);

      switch(event.type)
        {
        case SDL_QUIT:        /* Quit event - quit: */
          quit = 1;
          break;
        case SDL_KEYDOWN:     /* A keypress! */
          key = event.key.keysym.sym;

          if(tux.key_event(key,DOWN))
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
                      Menu::set_current(game_menu);
                      show_menu = 0;
                      st_pause_ticks_stop();
                    }
                  else
                    {
                      Menu::set_current(game_menu);
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

          if(tux.key_event(key, UP))
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
              if(debug_mode)
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
              if(debug_mode)
                distros += 50;
              break;
            case SDLK_SPACE:
              if(debug_mode)
                next_level = 1;
              break;
            case SDLK_DELETE:
              if(debug_mode)
                tux.got_coffee = 1;
              break;
            case SDLK_INSERT:
              if(debug_mode)
                timer_start(&tux.invincible_timer,TUX_INVINCIBLE_TIME);
              break;
            case SDLK_l:
              if(debug_mode)
                --tux.lives;
              break;
            case SDLK_s:
              if(debug_mode)
                score += 1000;
            case SDLK_f:
              if(debug_fps)
                debug_fps = false;
              else
                debug_fps = true;
              break;
            default:
              break;
            }
          break;

        case SDL_JOYAXISMOTION:
          switch(event.jaxis.axis)
            {
            case JOY_X:
              if (event.jaxis.value < -JOYSTICK_DEAD_ZONE)
                {
                  tux.input.left  = DOWN;
                  tux.input.right = UP;
                }
              else if (event.jaxis.value > JOYSTICK_DEAD_ZONE)
                {
                  tux.input.left  = UP;
                  tux.input.right = DOWN;
                }
              else
                {
                  tux.input.left  = DOWN;
                  tux.input.right = DOWN;
                }
              break;
            case JOY_Y:
              if (event.jaxis.value > JOYSTICK_DEAD_ZONE)
                tux.input.down = DOWN;
              else if (event.jaxis.value < -JOYSTICK_DEAD_ZONE)
                tux.input.down = UP;
              else
                tux.input.down = UP;
              
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
	    
          break;

        default:
          break;

        }  /* switch */

    } /* while */
}

int
GameSession::action()
{
  if (tux.is_dead() || next_level)
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
              world->get_level()->cleanup();
              level_free_song();
              unloadshared();
              world->arrays_free();
              return(0);
            }
          tux.level_begin();
        }
      else
        {
          tux.is_dying();

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
              world->get_level()->cleanup();
              level_free_song();
              unloadshared();
              world->arrays_free();
              return(0);
            } /* if (lives < 0) */
        }

      /* Either way, (re-)load the (next) level... */

      tux.level_begin();
      set_defaults();
      world->get_level()->cleanup();

      if (st_gl_mode == ST_GL_LOAD_LEVEL_FILE)
        {
          if(world->get_level()->load(level_subset) != 0)
            return 0;
        }
      else
        {
          if(world->get_level()->load(level_subset,level) != 0)
            return 0;
        }

      world->arrays_free();
      activate_bad_guys(world->get_level());
      world->activate_particle_systems();
      level_free_gfx();
      world->get_level()->load_gfx();
      level_free_song();
      world->get_level()->load_song();
      if(st_gl_mode != ST_GL_TEST)
        levelintro();
      start_timers();
      /* Play music: */
      play_current_music();
    }

  tux.action();

  world->action();

  /* update particle systems */
  std::vector<ParticleSystem*>::iterator p;
  for(p = world->particle_systems.begin(); p != world->particle_systems.end(); ++p)
    {
      (*p)->simulate(frame_ratio);
    }

  /* Handle all possible collisions. */
  collision_handler();

  return -1;
}

void 
GameSession::draw()
{

  
  world->draw();

  drawstatus();

  if(game_pause)
    {
      int x = screen->h / 20;
      for(int i = 0; i < x; ++i)
        {
          fillrect(i % 2 ? (pause_menu_frame * i)%screen->w : -((pause_menu_frame * i)%screen->w) ,(i*20+pause_menu_frame)%screen->h,screen->w,10,20,20,20, rand() % 20 + 1);
        }
      fillrect(0,0,screen->w,screen->h,rand() % 50, rand() % 50, rand() % 50, 128);
      text_drawf(&blue_text, "PAUSE - Press 'P' To Play", 0, 230, A_HMIDDLE, A_TOP, 1);
    }

  if(show_menu)
  {
    menu_process_current();
    mouse_cursor->draw();
  }

  /* (Update it all!) */
  updatescreen();
}


int
GameSession::run()
{
  current_ = this;
  
  int  fps_cnt;
  bool jump;
  bool done;

  /* --- MAIN GAME LOOP!!! --- */
  jump = false;
  done = false;
  quit = 0;
  global_frame_counter = 0;
  game_pause = 0;
  timer_init(&fps_timer,true);
  timer_init(&frame_timer,true);
  last_update_time = st_get_ticks();
  fps_cnt = 0;

  /* Clear screen: */
  clearscreen(0, 0, 0);
  updatescreen();

  /* Play music: */
  play_current_music();

  while (SDL_PollEvent(&event))
  {}

  draw();
  do
    {
      jump = false;

      /* Calculate the movement-factor */
      frame_ratio = ((double)(update_time-last_update_time))/((double)FRAME_RATE);
      if(frame_ratio > 1.5) /* Quick hack to correct the unprecise CPU clocks a little bit. */
        frame_ratio = 1.5 + (frame_ratio - 1.5) * 0.85;

      if(!timer_check(&frame_timer))
        {
          timer_start(&frame_timer,25);
          ++global_frame_counter;
        }

      /* Handle events: */

      tux.input.old_fire = tux.input.fire;

      process_events();

      if(show_menu)
        {
          if(current_menu == game_menu)
            {
              switch (game_menu->check())
                {
                case 2:
                  st_pause_ticks_stop();
                  break;
                case 3:
                  update_load_save_game_menu(save_game_menu, false);
                  break;
                case 4:
                  update_load_save_game_menu(load_game_menu, true);
                  break;
                case 7:
                  st_pause_ticks_stop();
                  done = true;
                  break;
                }
            }
          else if(current_menu == options_menu)
            {
              process_options_menu();
            }
          else if(current_menu == save_game_menu )
            {
              process_save_game_menu();
            }
          else if(current_menu == load_game_menu )
            {
              process_load_game_menu();
            }
        }


      /* Handle actions: */

      if(!game_pause && !show_menu)
        {
          /*float z = frame_ratio;
            frame_ratio = 1;
            while(z >= 1)
            {*/
          if (action() == 0)
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

      if(debug_mode && debug_fps)
        SDL_Delay(60);

      /*Draw the current scene to the screen */
      /*If the machine running the game is too slow
        skip the drawing of the frame (so the calculations are more precise and
        the FPS aren't affected).*/
      /*if( ! fps_fps < 50.0 )
        game_draw();
        else
        jump = true;*/ /*FIXME: Implement this tweak right.*/
      draw();

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
      if(last_update_time >= update_time - 12 && !jump) {
        SDL_Delay(10);
        update_time = st_get_ticks();
      }
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
        tux.kill(KILL);


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
  world->get_level()->cleanup();
  level_free_song();
  unloadshared();
  world->arrays_free();

  game_started = false;

  return(quit);
}

/* Draw a tile on the screen: */

void drawshape(float x, float y, unsigned int c, Uint8 alpha)
{
  if (c != 0)
    {
      Tile* ptile = TileManager::instance()->get(c);
      if(ptile)
        {
          if(ptile->images.size() > 1)
            {
              texture_draw(&ptile->images[( ((global_frame_counter*25) / ptile->anim_speed) % (ptile->images.size()))],x,y, alpha);
            }
          else if (ptile->images.size() == 1)
            {
              texture_draw(&ptile->images[0],x,y, alpha);
            }
          else
            {
              //printf("Tile not dravable %u\n", c);
            }
        }
    }
}

/* Bounce a brick: */
void bumpbrick(float x, float y)
{
  world.add_bouncy_brick(((int)(x + 1) / 32) * 32,
                         (int)(y / 32) * 32);

  play_sound(sounds[SND_BRICK], SOUND_CENTER_SPEAKER);
}

/* (Status): */
void drawstatus(void)
{
  char str[60];

  sprintf(str, "%d", score);
  text_draw(&white_text, "SCORE", 0, 0, 1);
  text_draw(&gold_text, str, 96, 0, 1);

  if(st_gl_mode != ST_GL_TEST)
    {
      sprintf(str, "%d", hs_score);
      text_draw(&white_text, "HIGH", 0, 20, 1);
      text_draw(&gold_text, str, 96, 20, 1);
    }
  else
    {
      text_draw(&white_text,"Press ESC To Return",0,20,1);
    }

  if (timer_get_left(&time_left) > TIME_WARNING || (global_frame_counter % 10) < 5)
    {
      sprintf(str, "%d", timer_get_left(&time_left) / 1000 );
      text_draw(&white_text, "TIME", 224, 0, 1);
      text_draw(&gold_text, str, 304, 0, 1);
    }

  sprintf(str, "%d", distros);
  text_draw(&white_text, "DISTROS", screen->h, 0, 1);
  text_draw(&gold_text, str, 608, 0, 1);

  text_draw(&white_text, "LIVES", screen->h, 20, 1);

  if(show_fps)
    {
      sprintf(str, "%2.1f", fps_fps);
      text_draw(&white_text, "FPS", screen->h, 40, 1);
      text_draw(&gold_text, str, screen->h + 60, 40, 1);
    }

  for(int i=0; i < tux.lives; ++i)
    {
      texture_draw(&tux_life,565+(18*i),20);
    }
}


void drawendscreen(void)
{
  char str[80];

  clearscreen(0, 0, 0);

  text_drawf(&blue_text, "GAMEOVER", 0, 200, A_HMIDDLE, A_TOP, 1);

  sprintf(str, "SCORE: %d", score);
  text_drawf(&gold_text, str, 0, 224, A_HMIDDLE, A_TOP, 1);

  sprintf(str, "DISTROS: %d", distros);
  text_drawf(&gold_text, str, 0, 256, A_HMIDDLE, A_TOP, 1);

  flipscreen();
  
  SDL_Event event;
  wait_for_event(event,2000,5000,true);
}

void drawresultscreen(void)
{
  char str[80];

  clearscreen(0, 0, 0);

  text_drawf(&blue_text, "Result:", 0, 200, A_HMIDDLE, A_TOP, 1);

  sprintf(str, "SCORE: %d", score);
  text_drawf(&gold_text, str, 0, 224, A_HMIDDLE, A_TOP, 1);

  sprintf(str, "DISTROS: %d", distros);
  text_drawf(&gold_text, str, 0, 256, A_HMIDDLE, A_TOP, 1);

  flipscreen();
  
  SDL_Event event;
  wait_for_event(event,2000,5000,true);
}

void
GameSession::savegame(int slot)
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
      fwrite(&tux,sizeof(Player),1,fi);
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

void
GameSession::loadgame(int slot)
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
      world->get_level()->cleanup();
      if(world->get_level()->load(level_subset,level) != 0)
        exit(1);
      world->arrays_free();
      activate_bad_guys(world->get_level());
      world->activate_particle_systems();
      level_free_gfx();
      world->get_level()->load_gfx();
      level_free_song();
      world->get_level()->load_song();
      levelintro();
      update_time = st_get_ticks();

      fread(&score,   sizeof(int),1,fi);
      fread(&distros, sizeof(int),1,fi);
      fread(&scroll_x,sizeof(float),1,fi);
      fread(&tux,     sizeof(Player), 1, fi);
      timer_fread(&tux.invincible_timer,fi);
      timer_fread(&tux.skidding_timer,fi);
      timer_fread(&tux.safe_timer,fi);
      timer_fread(&tux.frame_timer,fi);
      timer_fread(&time_left,fi);
      fread(&ui,sizeof(int),1,fi);
      fclose(fi);
    }

}

std::string slotinfo(int slot)
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

  return tmp;
}

