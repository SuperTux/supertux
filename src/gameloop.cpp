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
#include "resources.h"

GameSession* GameSession::current_ = 0;

void
GameSession::init()
{
  game_pause = false;
}

GameSession::GameSession()
{
  current_ = this;
  assert(0);
}

GameSession::GameSession(const std::string& filename)
{
  init();

  //assert(!"Don't call me");
  current_ = this;

  world = new World;

  fps_timer.init(true);
  frame_timer.init(true);

  world->load(filename);
}

GameSession::GameSession(const std::string& subset_, int levelnb_, int mode)
  : subset(subset_),
    levelnb(levelnb_)
{
  init();

  current_ = this;

  world = new World;

  fps_timer.init(true);
  frame_timer.init(true);

  st_gl_mode = mode;
  
  /* Init the game: */
  world->arrays_free();
  world->set_defaults();

  if (st_gl_mode == ST_GL_LOAD_LEVEL_FILE)
    {
      if (world->load(subset))
        exit(1);
    }
  else
    {
      if(world->load(subset, levelnb) != 0)
        exit(1);
    }

  world->get_level()->load_gfx();
  
  world->activate_bad_guys();
  world->activate_particle_systems();
  world->get_level()->load_song();

  if(st_gl_mode != ST_GL_TEST)
    load_hs();

  if(st_gl_mode == ST_GL_PLAY || st_gl_mode == ST_GL_LOAD_LEVEL_FILE)
    levelintro();

  time_left.init(true);
  start_timers();

  if(st_gl_mode == ST_GL_LOAD_GAME)
    loadgame(levelnb);
}

GameSession::~GameSession()
{
  delete world;
}

void
GameSession::levelintro(void)
{
  Player& tux = *world->get_tux();

  char str[60];
  /* Level Intro: */
  clearscreen(0, 0, 0);

  sprintf(str, "LEVEL %d", levelnb);
  blue_text->drawf(str, 0, 200, A_HMIDDLE, A_TOP, 1);

  sprintf(str, "%s", world->get_level()->name.c_str());
  gold_text->drawf(str, 0, 224, A_HMIDDLE, A_TOP, 1);

  sprintf(str, "by %s", world->get_level()->author.c_str());
  gold_text->drawf(str, 0, 256, A_HMIDDLE, A_TOP, 1);
  
  sprintf(str, "TUX x %d", tux.lives);
  white_text->drawf(str, 0, 288, A_HMIDDLE, A_TOP, 1);

  flipscreen();

  SDL_Event event;
  wait_for_event(event,1000,3000,true);
}

/* Reset Timers */
void
GameSession::start_timers()
{
  time_left.start(world->get_level()->time_left*1000);
  st_pause_ticks_init();
  update_time = st_get_ticks();
}

void
GameSession::process_events()
{
  Player& tux = *world->get_tux();

  SDL_Event event;
  while (SDL_PollEvent(&event))
    {
      /* Check for menu-events, if the menu is shown */
      if(show_menu)
        current_menu->event(event);

      switch(event.type)
        {
        case SDL_QUIT:        /* Quit event - quit: */
          quit = true;
          break;
        case SDL_KEYDOWN:     /* A keypress! */
          {
            SDLKey key = event.key.keysym.sym;
            
            if(tux.key_event(key,DOWN))
              break;

            switch(key)
              {
              case SDLK_ESCAPE:    /* Escape: Open/Close the menu: */
                if(!game_pause)
                  {
                    if(st_gl_mode == ST_GL_TEST)
                      quit = true;
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
          }
          break;
        case SDL_KEYUP:      /* A keyrelease! */
          {
            SDLKey key = event.key.keysym.sym;

            if(tux.key_event(key, UP))
              break;

            switch(key)
              {
              case SDLK_p:
                if(!show_menu)
                  {
                    if(game_pause)
                      {
                        game_pause = false;
                        st_pause_ticks_stop();
                      }
                    else
                      {
                        game_pause = true;
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
                  player_status.distros += 50;
                break;
              case SDLK_SPACE:
                if(debug_mode)
                  player_status.next_level = 1;
                break;
              case SDLK_DELETE:
                if(debug_mode)
                  tux.got_coffee = 1;
                break;
              case SDLK_INSERT:
                if(debug_mode)
                  tux.invincible_timer.start(TUX_INVINCIBLE_TIME);
                break;
              case SDLK_l:
                if(debug_mode)
                  --tux.lives;
                break;
              case SDLK_s:
                if(debug_mode)
                  player_status.score += 1000;
              case SDLK_f:
                if(debug_fps)
                  debug_fps = false;
                else
                  debug_fps = true;
                break;
              default:
                break;
              }
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
GameSession::action(double frame_ratio)
{
  Player& tux = *world->get_tux();

  if (tux.is_dead() || player_status.next_level)
    {
      /* Tux either died, or reached the end of a level! */
      halt_music();
      
      if (player_status.next_level)
        {
          /* End of a level! */
          levelnb++;
          player_status.next_level = 0;
          if(st_gl_mode != ST_GL_TEST)
            {
              drawresultscreen();
            }
          else
            {
              world->get_level()->free_gfx();
              world->get_level()->cleanup();
              world->get_level()->free_song();
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
                  if (player_status.score > hs_score)
                    save_hs(player_status.score);
                }

              world->get_level()->free_gfx();
              world->get_level()->cleanup();
              world->get_level()->free_song();
              world->arrays_free();

              return(0);
            } /* if (lives < 0) */
        }

      /* Either way, (re-)load the (next) level... */
      tux.level_begin();
      world->set_defaults();
      
      world->get_level()->cleanup();

      if (st_gl_mode == ST_GL_LOAD_LEVEL_FILE)
        {
          if(world->get_level()->load(subset) != 0)
            return 0;
        }
      else
        {
          if(world->get_level()->load(subset, levelnb) != 0)
            return 0;
        }

      world->arrays_free();
      world->activate_bad_guys();
      world->activate_particle_systems();

      world->get_level()->free_gfx();
      world->get_level()->load_gfx();
      world->get_level()->free_song();
      world->get_level()->load_song();

      if(st_gl_mode != ST_GL_TEST)
        levelintro();
      start_timers();
      /* Play music: */
      play_current_music();
    }

  tux.action(frame_ratio);

  world->action(frame_ratio);

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
      blue_text->drawf("PAUSE - Press 'P' To Play", 0, 230, A_HMIDDLE, A_TOP, 1);
    }

  if(show_menu)
    {
      menu_process_current();
      mouse_cursor->draw();
    }

  updatescreen();
}


int
GameSession::run()
{
  Player& tux = *world->get_tux();
  current_ = this;
  
  int  fps_cnt;
  bool done;

  global_frame_counter = 0;
  game_pause = false;

  fps_timer.init(true);
  frame_timer.init(true);

  last_update_time = st_get_ticks();
  fps_cnt = 0;

  /* Clear screen: */
  clearscreen(0, 0, 0);
  updatescreen();

  /* Play music: */
  play_current_music();

  // Eat unneeded events
  SDL_Event event;
  while (SDL_PollEvent(&event)) {}

  draw();

  done = false;
  quit = false;
  while (!done && !quit)
    {
      /* Calculate the movement-factor */
      double frame_ratio = ((double)(update_time-last_update_time))/((double)FRAME_RATE);
      if(frame_ratio > 1.5) /* Quick hack to correct the unprecise CPU clocks a little bit. */
        frame_ratio = 1.5 + (frame_ratio - 1.5) * 0.85;

      if(!frame_timer.check())
        {
          frame_timer.start(25);
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
          if (action(frame_ratio) == 0)
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
      if(last_update_time >= update_time - 12) {
        SDL_Delay(10);
        update_time = st_get_ticks();
      }
      /*if((update_time - last_update_time) < 10)
        SDL_Delay((11 - (update_time - last_update_time))/2);*/

      /* Handle time: */
      if (time_left.check())
        {
          /* are we low on time ? */
          if (time_left.get_left() < TIME_WARNING
              && (get_current_music() != HURRYUP_MUSIC))     /* play the fast music */
            {
              set_current_music(HURRYUP_MUSIC);
              play_current_music();
            }

        }
      else if(tux.dying == DYING_NOT)
        tux.kill(KILL);

      /* Calculate frames per second */
      if(show_fps)
        {
          ++fps_cnt;
          fps_fps = (1000.0 / (float)fps_timer.get_gone()) * (float)fps_cnt;

          if(!fps_timer.check())
            {
              fps_timer.start(1000);
              fps_cnt = 0;
            }
        }
    }

  halt_music();

  world->get_level()->free_gfx();
  world->get_level()->cleanup();
  world->get_level()->free_song();

  world->arrays_free();

  return quit;
}

/* Bounce a brick: */
void bumpbrick(float x, float y)
{
  World::current()->add_bouncy_brick(((int)(x + 1) / 32) * 32,
                         (int)(y / 32) * 32);

  play_sound(sounds[SND_BRICK], SOUND_CENTER_SPEAKER);
}

/* (Status): */
void
GameSession::drawstatus()
{
  Player& tux = *world->get_tux();
  char str[60];

  sprintf(str, "%d", player_status.score);
  white_text->draw("SCORE", 0, 0, 1);
  gold_text->draw(str, 96, 0, 1);

  if(st_gl_mode != ST_GL_TEST)
    {
      sprintf(str, "%d", hs_score);
      white_text->draw("HIGH", 0, 20, 1);
      gold_text->draw(str, 96, 20, 1);
    }
  else
    {
      white_text->draw("Press ESC To Return",0,20,1);
    }

  if (time_left.get_left() > TIME_WARNING || (global_frame_counter % 10) < 5)
    {
      sprintf(str, "%d", time_left.get_left() / 1000 );
      white_text->draw("TIME", 224, 0, 1);
      gold_text->draw(str, 304, 0, 1);
    }

  sprintf(str, "%d", player_status.distros);
  white_text->draw("DISTROS", screen->h, 0, 1);
  gold_text->draw(str, 608, 0, 1);

  white_text->draw("LIVES", screen->h, 20, 1);

  if(show_fps)
    {
      sprintf(str, "%2.1f", fps_fps);
      white_text->draw("FPS", screen->h, 40, 1);
      gold_text->draw(str, screen->h + 60, 40, 1);
    }

  for(int i= 0; i < tux.lives; ++i)
    {
      tux_life->draw(565+(18*i),20);
    }
}

void
GameSession::drawendscreen()
{
  char str[80];

  clearscreen(0, 0, 0);

  blue_text->drawf("GAMEOVER", 0, 200, A_HMIDDLE, A_TOP, 1);

  sprintf(str, "SCORE: %d", player_status.score);
  gold_text->drawf(str, 0, 224, A_HMIDDLE, A_TOP, 1);

  sprintf(str, "DISTROS: %d", player_status.distros);
  gold_text->drawf(str, 0, 256, A_HMIDDLE, A_TOP, 1);

  flipscreen();
  
  SDL_Event event;
  wait_for_event(event,2000,5000,true);
}

void
GameSession::drawresultscreen(void)
{
  char str[80];

  clearscreen(0, 0, 0);

  blue_text->drawf("Result:", 0, 200, A_HMIDDLE, A_TOP, 1);

  sprintf(str, "SCORE: %d", player_status.score);
  gold_text->drawf(str, 0, 224, A_HMIDDLE, A_TOP, 1);

  sprintf(str, "DISTROS: %d", player_status.distros);
  gold_text->drawf(str, 0, 256, A_HMIDDLE, A_TOP, 1);

  flipscreen();
  
  SDL_Event event;
  wait_for_event(event,2000,5000,true);
}

void
GameSession::savegame(int)
{
#if 0
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
      //FIXME:fwrite(&tux,sizeof(Player),1,fi);
      //FIXME:timer_fwrite(&tux.invincible_timer,fi);
      //FIXME:timer_fwrite(&tux.skidding_timer,fi);
      //FIXME:timer_fwrite(&tux.safe_timer,fi);
      //FIXME:timer_fwrite(&tux.frame_timer,fi);
      timer_fwrite(&time_left,fi);
      ui = st_get_ticks();
      fwrite(&ui,sizeof(int),1,fi);
    }
  fclose(fi);
#endif 
}

void
GameSession::loadgame(int)
{
#if 0
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

      world->set_defaults();
      world->get_level()->cleanup();
      world->arrays_free();
      world->get_level()->free_gfx();
      world->get_level()->free_song();

      if(world->get_level()->load(level_subset,level) != 0)
        exit(1);

      world->activate_bad_guys();
      world->activate_particle_systems();
      world->get_level()->load_gfx();
      world->get_level()->load_song();

      levelintro();
      update_time = st_get_ticks();

      fread(&score,   sizeof(int),1,fi);
      fread(&distros, sizeof(int),1,fi);
      fread(&scroll_x,sizeof(float),1,fi);
      //FIXME:fread(&tux,     sizeof(Player), 1, fi);
      //FIXME:timer_fread(&tux.invincible_timer,fi);
      //FIXME:timer_fread(&tux.skidding_timer,fi);
      //FIXME:timer_fread(&tux.safe_timer,fi);
      //FIXME:timer_fread(&tux.frame_timer,fi);
      timer_fread(&time_left,fi);
      fread(&ui,sizeof(int),1,fi);
      fclose(fi);
    }
#endif 
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

