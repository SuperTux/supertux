//  $Id$
// 
//  SuperTux
//  Copyright (C) 2000 Bill Kendrick <bill@newbreedsoftware.com>
//  Copyright (C) 2004 Tobias Glaesser <tobi.web@gmx.de>
//  Copyright (C) 2004 Ingo Ruhnke <grumbel@gmx.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include <iostream>
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
#include "music_manager.h"

GameSession* GameSession::current_ = 0;

GameSession::GameSession(const std::string& subset_, int levelnb_, int mode)
  : world(0), st_gl_mode(mode), levelnb(levelnb_), end_sequence(false),
    subset(subset_)
{
  current_ = this;
  
  global_frame_counter = 0;
  game_pause = false;

  fps_timer.init(true);            
  frame_timer.init(true);

  restart_level();
}

void
GameSession::restart_level()
{
  game_pause   = false;
  exit_status  = NONE;
  end_sequence = false;

  fps_timer.init(true);
  frame_timer.init(true);

  float old_x_pos = -1;

  if (world)
    { // Tux has lost a life, so we try to respawn him at the nearest reset point
      old_x_pos = world->get_tux()->base.x;
    }
  
  delete world;

  if (st_gl_mode == ST_GL_LOAD_LEVEL_FILE)
    {
      world = new World(subset);
    }
  else if (st_gl_mode == ST_GL_DEMO_GAME)
    {
      world = new World(subset);
    }
  else
    {
      world = new World(subset, levelnb);
    }

  // Set Tux to the nearest reset point
  if (old_x_pos != -1)
    {
      ResetPoint best_reset_point = { -1, -1 };
      for(std::vector<ResetPoint>::iterator i = get_level()->reset_points.begin();
          i != get_level()->reset_points.end(); ++i)
        {
          if (i->x < old_x_pos && best_reset_point.x < i->x)
            best_reset_point = *i;
        }
      
      if (best_reset_point.x != -1)
        {
          world->get_tux()->base.x = best_reset_point.x;
          world->get_tux()->base.y = best_reset_point.y;
        }
    }

    
  if (st_gl_mode != ST_GL_DEMO_GAME)
    {
      if(st_gl_mode == ST_GL_PLAY || st_gl_mode == ST_GL_LOAD_LEVEL_FILE)
        levelintro();
    }

  time_left.init(true);
  start_timers();
  world->play_music(LEVEL_MUSIC);
}

GameSession::~GameSession()
{
  delete world;
}

void
GameSession::levelintro(void)
{
  music_manager->halt_music();
  
  char str[60];
 
  if (get_level()->img_bkgd)
    get_level()->img_bkgd->draw(0, 0);
  else
    drawgradient(get_level()->bkgd_top, get_level()->bkgd_bottom);

  sprintf(str, "%s", world->get_level()->name.c_str());
  gold_text->drawf(str, 0, 200, A_HMIDDLE, A_TOP, 1);

  sprintf(str, "TUX x %d", player_status.lives);
  white_text->drawf(str, 0, 224, A_HMIDDLE, A_TOP, 1);
  
  sprintf(str, "by %s", world->get_level()->author.c_str());
  white_small_text->drawf(str, 0, 400, A_HMIDDLE, A_TOP, 1);
  

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
GameSession::on_escape_press()
{
  if(game_pause)
    return;

  if(st_gl_mode == ST_GL_TEST)
    {
      exit_status = LEVEL_ABORT;
    }
  else if (!Menu::current())
    {
      Menu::set_current(game_menu);
    }
}

void
GameSession::process_events()
{
  if (end_sequence)
    {
      Player& tux = *world->get_tux();
          
      tux.input.left  = UP;
      tux.input.right = DOWN; 
      tux.input.down  = UP; 

      if (int(last_x_pos) == int(tux.base.x))
        tux.input.up    = DOWN; 
      else
        tux.input.up    = UP; 

      last_x_pos = tux.base.x;
    }
  else
    {
      if(!Menu::current() && !game_pause)
        st_pause_ticks_stop();

      SDL_Event event;
      while (SDL_PollEvent(&event))
        {
          /* Check for menu-events, if the menu is shown */
          if (Menu::current())
            {
              Menu::current()->event(event);
              st_pause_ticks_start();
            }
          else
            {
              Player& tux = *world->get_tux();
  
              switch(event.type)
                {
                case SDL_QUIT:        /* Quit event - quit: */
                  st_abort("Received window close", "");
                  break;

                case SDL_KEYDOWN:     /* A keypress! */
                  {
                    SDLKey key = event.key.keysym.sym;
            
                    if(tux.key_event(key,DOWN))
                      break;

                    switch(key)
                      {
                      case SDLK_ESCAPE:    /* Escape: Open/Close the menu: */
                        on_escape_press();
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
                        if(!Menu::current())
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
                          --player_status.lives;
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
                  if (event.jaxis.axis == joystick_keymap.x_axis)
                    {
                      if (event.jaxis.value < -joystick_keymap.dead_zone)
                        {
                          tux.input.left  = DOWN;
                          tux.input.right = UP;
                        }
                      else if (event.jaxis.value > joystick_keymap.dead_zone)
                        {
                          tux.input.left  = UP;
                          tux.input.right = DOWN;
                        }
                      else
                        {
                          tux.input.left  = DOWN;
                          tux.input.right = DOWN;
                        }
                    }
                  else if (event.jaxis.axis == joystick_keymap.y_axis)
                    {
                      if (event.jaxis.value > joystick_keymap.dead_zone)
                        tux.input.down = DOWN;
                      else if (event.jaxis.value < -joystick_keymap.dead_zone)
                        tux.input.down = UP;
                      else
                        tux.input.down = UP;
                    }
                  break;
            
                case SDL_JOYBUTTONDOWN:
                  if (event.jbutton.button == joystick_keymap.a_button)
                    tux.input.up = DOWN;
                  else if (event.jbutton.button == joystick_keymap.b_button)
                    tux.input.fire = DOWN;
                  else if (event.jbutton.button == joystick_keymap.start_button)
                    on_escape_press();
                  break;
                case SDL_JOYBUTTONUP:
                  if (event.jbutton.button == joystick_keymap.a_button)
                    tux.input.up = UP;
                  else if (event.jbutton.button == joystick_keymap.b_button)
                    tux.input.fire = UP;
                  break;

                default:
                  break;
                }  /* switch */
            }
        } /* while */
    }
}


void
GameSession::check_end_conditions()
{
  Player* tux = world->get_tux();

  /* End of level? */
  if (tux->base.x >= World::current()->get_level()->endpos + 32 * (get_level()->use_endsequence ? 22 : 10))
    {
      exit_status = LEVEL_FINISHED;
    }
  else if (tux->base.x >= World::current()->get_level()->endpos && !end_sequence)
    {
      end_sequence = true;
      last_x_pos = -1;
      music_manager->halt_music();
    }
  else
    {
      // Check End conditions
      if (tux->is_dead())
        {
          player_status.lives -= 1;             
    
          if (player_status.lives < 0)
            { // No more lives!?
              if(st_gl_mode != ST_GL_TEST)
                drawendscreen();
              
              exit_status = GAME_OVER;
            }
          else
            { // Still has lives, so reset Tux to the levelstart
              restart_level();
            }
        }
    } 
}

void
GameSession::action(double frame_ratio)
{
  check_end_conditions();
  
  if (exit_status == NONE)
    {
      // Update Tux and the World
      world->action(frame_ratio);
    }
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

  if(Menu::current())
    {
      Menu::current()->draw();
      mouse_cursor->draw();
    }

  updatescreen();
}

void
GameSession::process_menu()
{
  Menu* menu = Menu::current();
  if(menu)
    {
      menu->action();

      if(menu == game_menu)
        {
          switch (game_menu->check())
            {
            case MNID_CONTINUE:
              st_pause_ticks_stop();
              break;
            case MNID_ABORTLEVEL:
              st_pause_ticks_stop();
              exit_status = LEVEL_ABORT;
              break;
            }
        }
      else if(menu == options_menu)
        {
          process_options_menu();
        }
      else if(menu == load_game_menu )
        {
          process_load_game_menu();
        }
    }
}

GameSession::ExitStatus
GameSession::run()
{
  Menu::set_current(0);
  current_ = this;
  
  int fps_cnt = 0;

  update_time = last_update_time = st_get_ticks();

  /* Clear screen: */
  clearscreen(0, 0, 0);
  updatescreen();

  // Eat unneeded events
  SDL_Event event;
  while (SDL_PollEvent(&event)) {}

  draw();

  float overlap = 0.0f;
  while (exit_status == NONE)
    {
      /* Calculate the movement-factor */
      double frame_ratio = ((double)(update_time-last_update_time))/((double)FRAME_RATE);

      if(!frame_timer.check())
        {
          frame_timer.start(25);
          ++global_frame_counter;
        }

      /* Handle events: */
      world->get_tux()->input.old_fire = world->get_tux()->input.fire;

      process_events();
      process_menu();

      // Update the world state and all objects in the world
      // Do that with a constante time-delta so that the game will run
      // determistic and not different on different machines
      if(!game_pause && !Menu::current())
        {
          frame_ratio *= game_speed;
          frame_ratio += overlap;
          while (frame_ratio > 0)
            {
              // Update the world
              if (end_sequence)
                action(.5f);
              else
                action(1.0f);
              frame_ratio -= 1.0f;
            }
          overlap = frame_ratio;
        }
      else
        {
          ++pause_menu_frame;
          SDL_Delay(50);
        }

      draw();

      /* Time stops in pause mode */
      if(game_pause || Menu::current())
        {
          continue;
        }

      /* Set the time of the last update and the time of the current update */
      last_update_time = update_time;
      update_time      = st_get_ticks();

      /* Pause till next frame, if the machine running the game is too fast: */
      /* FIXME: Works great for in OpenGl mode, where the CPU doesn't have to do that much. But
         the results in SDL mode aren't perfect (thought the 100 FPS are reached), even on an AMD2500+. */
      if(last_update_time >= update_time - 12) 
        {
          SDL_Delay(10);
          update_time = st_get_ticks();
        }

      /* Handle time: */
      if (!time_left.check() && world->get_tux()->dying == DYING_NOT)
        world->get_tux()->kill(Player::KILL);

      /* Handle music: */
      if(world->get_tux()->invincible_timer.check())
        {
          if(world->get_music_type() != HERRING_MUSIC)
            world->play_music(HERRING_MUSIC);
        }
      /* are we low on time ? */
      else if (time_left.get_left() < TIME_WARNING
         && (world->get_music_type() == LEVEL_MUSIC))
        {
          world->play_music(HURRYUP_MUSIC);
        }
      /* or just normal music? */
      else if(world->get_music_type() != LEVEL_MUSIC)
        {
          world->play_music(LEVEL_MUSIC);
        }

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
  
  return exit_status;
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
  char str[60];

  sprintf(str, "%d", player_status.score);
  white_text->draw("SCORE", 0, 0, 1);
  gold_text->draw(str, 96, 0, 1);

  if(st_gl_mode == ST_GL_TEST)
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
  white_text->draw("COINS", screen->h, 0, 1);
  gold_text->draw(str, 608, 0, 1);

  white_text->draw("LIVES", screen->h, 20, 1);

  if(show_fps)
    {
      sprintf(str, "%2.1f", fps_fps);
      white_text->draw("FPS", screen->h, 40, 1);
      gold_text->draw(str, screen->h + 60, 40, 1);
    }

  for(int i= 0; i < player_status.lives; ++i)
    {
      tux_life->draw(565+(18*i),20);
    }
}

void
GameSession::drawendscreen()
{
  char str[80];

  if (get_level()->img_bkgd)
    get_level()->img_bkgd->draw(0, 0);
  else
    drawgradient(get_level()->bkgd_top, get_level()->bkgd_bottom);

  blue_text->drawf("GAMEOVER", 0, 200, A_HMIDDLE, A_TOP, 1);

  sprintf(str, "SCORE: %d", player_status.score);
  gold_text->drawf(str, 0, 224, A_HMIDDLE, A_TOP, 1);

  sprintf(str, "COINS: %d", player_status.distros);
  gold_text->drawf(str, 0, 256, A_HMIDDLE, A_TOP, 1);

  flipscreen();
  
  SDL_Event event;
  wait_for_event(event,2000,5000,true);
}

void
GameSession::drawresultscreen(void)
{
  char str[80];

  if (get_level()->img_bkgd)
    get_level()->img_bkgd->draw(0, 0);
  else
    drawgradient(get_level()->bkgd_top, get_level()->bkgd_bottom);

  blue_text->drawf("Result:", 0, 200, A_HMIDDLE, A_TOP, 1);

  sprintf(str, "SCORE: %d", player_status.score);
  gold_text->drawf(str, 0, 224, A_HMIDDLE, A_TOP, 1);

  sprintf(str, "COINS: %d", player_status.distros);
  gold_text->drawf(str, 0, 256, A_HMIDDLE, A_TOP, 1);

  flipscreen();
  
  SDL_Event event;
  wait_for_event(event,2000,5000,true);
}

std::string slotinfo(int slot)
{
  char tmp[1024];
  char slotfile[1024];
  sprintf(slotfile,"%s/slot%d.stsg",st_save_dir,slot);

  if (access(slotfile, F_OK) == 0)
    sprintf(tmp,"Slot %d - Savegame",slot);
  else
    sprintf(tmp,"Slot %d - Free",slot);

  return tmp;
}


