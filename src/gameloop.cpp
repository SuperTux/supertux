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
#include <sstream>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <ctime>

#include "SDL.h"

#ifndef WIN32
#include <sys/types.h>
#include <ctype.h>
#endif

#include "defines.h"
#include "app/globals.h"
#include "gameloop.h"
#include "video/screen.h"
#include "app/setup.h"
#include "high_scores.h"
#include "gui/menu.h"
#include "badguy.h"
#include "sector.h"
#include "special.h"
#include "player.h"
#include "level.h"
#include "scene.h"
#include "collision.h"
#include "tile.h"
#include "particlesystem.h"
#include "resources.h"
#include "background.h"
#include "tilemap.h"
#include "app/gettext.h"
#include "worldmap.h"
#include "intro.h"
#include "misc.h"
#include "camera.h"
#include "statistics.h"

GameSession* GameSession::current_ = 0;

bool compare_last(std::string& haystack, std::string needle)
{
int haystack_size = haystack.size();
int needle_size = needle.size();

if(haystack_size < needle_size)
  return false;

if(haystack.compare(haystack_size-needle_size, needle_size, needle) == 0)
  return true;
return false;
}

GameSession::GameSession(const std::string& levelname_, int mode, bool flip_level_, Statistics* statistics)
  : level(0), currentsector(0), st_gl_mode(mode),
    end_sequence(NO_ENDSEQUENCE), levelname(levelname_), flip_level(flip_level_),
    best_level_statistics(statistics)
{
  current_ = this;
  
  global_frame_counter = 0;
  game_pause = false;
  fps_fps = 0;

  fps_timer.init(true);
  frame_timer.init(true);
  random_timer.init(true);
  frame_rate.set_fps(100);

  context = new DrawingContext();

  if(flip_levels_mode)
    flip_level = true;

  restart_level();
}

void
GameSession::restart_level()
{
  game_pause   = false;
  exit_status  = ES_NONE;
  end_sequence = NO_ENDSEQUENCE;

  fps_timer.init(true);
  frame_timer.init(true);
  random_timer.init(true);

  last_keys.clear();

  Vector tux_pos = Vector(-1,-1);
  if (currentsector)
    { // Tux has lost a life, so we try to respawn him at the nearest reset point
      tux_pos = currentsector->player->base;
    }
  
  delete level;
  currentsector = 0;

  level = new Level;
  level->load(levelname);
  if(flip_level)
    level->do_vertical_flip();

  currentsector = level->get_sector("main");
  if(!currentsector)
    Termination::abort("Level has no main sector.", "");
  currentsector->activate("main");

  // Set Tux to the nearest reset point
  if(tux_pos.x != -1)
    {
    tux_pos = currentsector->get_best_spawn_point(tux_pos);
    currentsector->player->base.x = tux_pos.x;
    currentsector->player->base.y = tux_pos.y;
    
    // has to reset camera on swapping
    currentsector->camera->reset(Vector(currentsector->player->base.x,
                                        currentsector->player->base.y));
    }

  if (st_gl_mode != ST_GL_DEMO_GAME)
    {
      if(st_gl_mode == ST_GL_PLAY || st_gl_mode == ST_GL_LOAD_LEVEL_FILE)
        levelintro();
    }

  global_stats.reset();

  time_left.init(true);
  start_timers();
  currentsector->play_music(LEVEL_MUSIC);
}

GameSession::~GameSession()
{
  delete level;
  delete context;
}

void
GameSession::levelintro(void)
{
  SoundManager::get()->halt_music();
  
  char str[60];

  DrawingContext context;
  currentsector->background->draw(context);

  context.draw_text(gold_text, level->get_name(), Vector(screen->w/2, 160),
      CENTER_ALLIGN, LAYER_FOREGROUND1);

  sprintf(str, "TUX x %d", player_status.lives);
  context.draw_text(white_text, str, Vector(screen->w/2, 210),
      CENTER_ALLIGN, LAYER_FOREGROUND1);

  if(level->get_author().size())
    context.draw_text(white_small_text,
      std::string(_("by ")) + level->get_author(), 
      Vector(screen->w/2, 350), CENTER_ALLIGN, LAYER_FOREGROUND1);


  if(flip_level)
    context.draw_text(white_text,
      _("Level Vertically Flipped!"),
      Vector(screen->w/2, 310), CENTER_ALLIGN, LAYER_FOREGROUND1);

  if(best_level_statistics != NULL)
    best_level_statistics->draw_message_info(context, _("Best Level Statistics"));

  context.do_drawing();

  SDL_Event event;
  wait_for_event(event,1000,3000,true);
}

/* Reset Timers */
void
GameSession::start_timers()
{
  time_left.start(level->time_left*1000);
  Ticks::pause_init();
  frame_rate.start();
}

void
GameSession::on_escape_press()
{
  if(currentsector->player->dying || end_sequence != NO_ENDSEQUENCE)
    return;   // don't let the player open the menu, when he is dying
  
  if(game_pause)
    return;

  if(st_gl_mode == ST_GL_TEST)
    {
      exit_status = ES_LEVEL_ABORT;
    }
  else if (!Menu::current())
    {
      /* Tell Tux that the keys are all down, otherwise
        it could have nasty bugs, like going allways to the right
        or whatever that key does */
      Player& tux = *(currentsector->player);
      tux.key_event((SDLKey)keymap.jump, UP);
      tux.key_event((SDLKey)keymap.duck, UP);
      tux.key_event((SDLKey)keymap.left, UP);
      tux.key_event((SDLKey)keymap.right, UP);
      tux.key_event((SDLKey)keymap.fire, UP);

      Menu::set_current(game_menu);
      Ticks::pause_start();
    }
}

void
GameSession::process_events()
{
  if (end_sequence != NO_ENDSEQUENCE)
    {
      Player& tux = *currentsector->player;
         
      tux.input.fire  = UP;
      tux.input.left  = UP;
      tux.input.right = DOWN;
      tux.input.down  = UP; 

      if (int(last_x_pos) == int(tux.base.x))
        tux.input.up    = DOWN; 
      else
        tux.input.up    = UP; 

      last_x_pos = tux.base.x;

      SDL_Event event;
      while (SDL_PollEvent(&event))
        {
          /* Check for menu-events, if the menu is shown */
          if (Menu::current())
            {
              Menu::current()->event(event);
	      if(!Menu::current())
	      Ticks::pause_stop();
            }

          switch(event.type)
            {
            case SDL_QUIT:        /* Quit event - quit: */
              Termination::abort("Received window close", "");
              break;
              
            case SDL_KEYDOWN:     /* A keypress! */
              {
                SDLKey key = event.key.keysym.sym;
           
                switch(key)
                  {
                  case SDLK_ESCAPE:    /* Escape: Open/Close the menu: */
                    on_escape_press();
                    break;
                  default:
                    break;
                  }
              }
          
            case SDL_JOYBUTTONDOWN:
              if (event.jbutton.button == joystick_keymap.start_button)
                on_escape_press();
              break;
            }
        }
    }
  else // normal mode
    {
      if(!Menu::current() && !game_pause)
        Ticks::pause_stop();

      SDL_Event event;
      while (SDL_PollEvent(&event))
        {
          /* Check for menu-events, if the menu is shown */
          if (Menu::current())
            {
              Menu::current()->event(event);
              if(!Menu::current())
                Ticks::pause_stop();
            }
          else
            {
              Player& tux = *currentsector->player;
  
              switch(event.type)
                {
                case SDL_QUIT:        /* Quit event - quit: */
                  Termination::abort("Received window close", "");
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
                      case SDLK_a:
                        if(debug_mode)
                        {
                          char buf[160];
                          snprintf(buf, sizeof(buf), "P: %4.1f,%4.1f",
                              tux.base.x, tux.base.y);
                          context->draw_text(white_text, buf,
                              Vector(0, screen->h - white_text->get_height()),
                              LEFT_ALLIGN, LAYER_FOREGROUND1);
                          context->do_drawing();
                          SDL_Delay(1000);
                        }
                        break;
                      case SDLK_p:
                        if(!Menu::current())
                          {
                          // "lifeup" cheat activates pause cause of the 'p'
                          // so work around to ignore it
                            if(compare_last(last_keys, "lifeu"))
                              break;

                            if(game_pause)
                              {
                                game_pause = false;
                                Ticks::pause_stop();
                              }
                            else
                              {
                                game_pause = true;
                                Ticks::pause_start();
                              }
                          }
                        break;
                      default:
                        break;
                      }
                  }

                        /* Check if chacrater is ASCII */
                        char ch[2];
                        if((event.key.keysym.unicode & 0xFF80) == 0)
                          {
                          ch[0] = event.key.keysym.unicode & 0x7F;
                          ch[1] = '\0';
                          }
                        last_keys.append(ch);  // add to cheat keys

                        // Cheating words (the goal of this is really for debugging,
                        // but could be used for some cheating, nothing wrong with that)
                        if(compare_last(last_keys, "grow"))
                          {
                          tux.grow(false);
                          last_keys.clear();
                          }
                        if(compare_last(last_keys, "fire"))
                          {
                          tux.grow(false);
                          tux.got_power = tux.FIRE_POWER;
                          last_keys.clear();
                          }
                        if(compare_last(last_keys, "ice"))
                          {
                          tux.grow(false);
                          tux.got_power = tux.ICE_POWER;
                          last_keys.clear();
                          }
                        if(compare_last(last_keys, "lifeup"))
                          {
                          player_status.lives++;
                          last_keys.clear();
                          }
                        if(compare_last(last_keys, "lifedown"))
                          {
                          player_status.lives--;
                          last_keys.clear();
                          }
                        if(compare_last(last_keys, "invincible"))
                          {    // be invincle for the rest of the level
                          tux.invincible_timer.start(time_left.get_left());
                          last_keys.clear();
                          }
                        if(compare_last(last_keys, "shrink"))
                          {    // remove powerups
                          tux.kill(tux.SHRINK);
                          last_keys.clear();
                          }
                        if(compare_last(last_keys, "kill"))
                          {    // kill Tux
                          tux.kill(tux.KILL);
                          last_keys.clear();
                          }
                        if(compare_last(last_keys, "hover"))
                          {    // toggle hover ability on/off
                          tux.enable_hover = !tux.enable_hover;
                          last_keys.clear();
                          }
                        if(compare_last(last_keys, "gotoend"))
                          {    // goes to the end of the level
                          tux.base.x = (currentsector->solids->get_width()*32) - (screen->w*2);
                          tux.base.y = 0;
                          currentsector->camera->reset(Vector(tux.base.x, tux.base.y));
                          last_keys.clear();
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

                case SDL_JOYHATMOTION:
                  if(event.jhat.value & SDL_HAT_UP) {
                    tux.input.up = DOWN;
                    tux.input.down = UP;
                  } else if(event.jhat.value & SDL_HAT_DOWN) {
                    tux.input.up = UP;
                    tux.input.down = DOWN;
                  } else if(event.jhat.value & SDL_HAT_LEFT) {
                    tux.input.left = DOWN;
                    tux.input.right = UP;
                  } else if(event.jhat.value & SDL_HAT_RIGHT) {
                    tux.input.left = UP;
                    tux.input.right = DOWN;
                  } else if(event.jhat.value == SDL_HAT_CENTERED) {
                    tux.input.left = UP;
                    tux.input.right = UP;
                    tux.input.up = UP;
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
  Player* tux = currentsector->player;

  /* End of level? */
  Tile* endtile = collision_goal(tux->base);

  if(end_sequence && !endsequence_timer.check())
    {
      exit_status = ES_LEVEL_FINISHED;
      return;
    }
  else if(end_sequence == ENDSEQUENCE_RUNNING && endtile && endtile->data >= 1)
    {
      end_sequence = ENDSEQUENCE_WAITING;
    }
  else if(!end_sequence && endtile && endtile->data == 0)
    {
      end_sequence = ENDSEQUENCE_RUNNING;
      endsequence_timer.start(7000); // 5 seconds until we finish the map
      last_x_pos = -1;
      SoundManager::get()->play_music(level_end_song, 0);
      tux->invincible_timer.start(7000); //FIXME: Implement a winning timer for the end sequence (with special winning animation etc.)

      // add left time to stats
      global_stats.set_points(TIME_NEEDED_STAT, time_left.get_gone() / 1000);

      random_timer.start(200);  // start 1st firework
    }
  else if (!end_sequence && tux->is_dead())
    {
      player_status.bonus = PlayerStatus::NO_BONUS;

      if (player_status.lives < 0)
        { // No more lives!?
          exit_status = ES_GAME_OVER;
        }
      else
        { // Still has lives, so reset Tux to the levelstart
          restart_level();
        }

      return;
    }
}

void
GameSession::action(double frame_ratio)
{
  if (exit_status == ES_NONE && !currentsector->player->growing_timer.check())
    {
      // Update Tux and the World
      currentsector->action(frame_ratio);
    }

  // respawning in new sector?
  if(newsector != "" && newspawnpoint != "") {
    Sector* sector = level->get_sector(newsector);
    currentsector = sector;
    currentsector->activate(newspawnpoint);
    currentsector->play_music(LEVEL_MUSIC);
    newsector = newspawnpoint = "";
  }

  // on end sequence make a few fireworks
  if(end_sequence == ENDSEQUENCE_RUNNING && !random_timer.check() &&
     currentsector->end_sequence_animation() == FIREWORKS_ENDSEQ_ANIM)
    {
    Vector epicenter = currentsector->camera->get_translation();
    epicenter.x += screen->w * ((float)rand() / RAND_MAX);
    epicenter.y += (screen->h/2) * ((float)rand() / RAND_MAX);

    int red = rand() % 255;  // calculate firework color
    int green = rand() % red;
    currentsector->add_particles(epicenter, Vector(1.4,1.4), Vector(0,0),
                                 45, Color(red,green,0), 3, 1300);

    SoundManager::get()->play_sound(IDToSound(SND_FIREWORKS));
    random_timer.start(rand() % 400 + 600);  // next firework
    }
}

void 
GameSession::draw()
{
  currentsector->draw(*context);
  drawstatus(*context);

  if(game_pause)
    {
      int x = screen->h / 20;
      for(int i = 0; i < x; ++i)
        {
          context->draw_filled_rect(
              Vector(i % 2 ? (pause_menu_frame * i)%screen->w :
                -((pause_menu_frame * i)%screen->w)
                ,(i*20+pause_menu_frame)%screen->h),
              Vector(screen->w,10),
              Color(20,20,20, rand() % 20 + 1), LAYER_FOREGROUND1+1);
        }
      context->draw_filled_rect(
          Vector(0,0), Vector(screen->w, screen->h),
          Color(rand() % 50, rand() % 50, rand() % 50, 128), LAYER_FOREGROUND1);
      context->draw_text(blue_text, _("PAUSE - Press 'P' To Play"),
          Vector(screen->w/2, 230), CENTER_ALLIGN, LAYER_FOREGROUND1+2);

      char str1[60];
      char str2[124];
      sprintf(str1, _("Playing: "));
      sprintf(str2, level->name.c_str());

      context->draw_text(blue_text, str1,
          Vector((screen->w - (blue_text->get_text_width(str1) + white_text->get_text_width(str2)))/2, 340),
          LEFT_ALLIGN, LAYER_FOREGROUND1+2);
      context->draw_text(white_text, str2,
          Vector(((screen->w - (blue_text->get_text_width(str1) + white_text->get_text_width(str2)))/2)+blue_text->get_text_width(str1), 340),
          LEFT_ALLIGN, LAYER_FOREGROUND1+2);
    }

  if(Menu::current())
    {
      Menu::current()->draw(*context);
      mouse_cursor->draw(*context);
    }

  context->do_drawing();
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
              Ticks::pause_stop();
              break;
            case MNID_ABORTLEVEL:
              Ticks::pause_stop();
              exit_status = ES_LEVEL_ABORT;
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

  frame_rate.start();

  // Eat unneeded events
  SDL_Event event;
  while (SDL_PollEvent(&event)) {}

  draw();

  while (exit_status == ES_NONE)
    {
      /* Calculate the movement-factor */
      double frame_ratio = frame_rate.get();

      if(!frame_timer.check())
        {
          frame_timer.start(25);
          ++global_frame_counter;
        }

      /* Handle events: */
      currentsector->player->input.old_fire 
        = currentsector->player->input.fire;

      process_events();
      process_menu();

      // Update the world state and all objects in the world
      // Do that with a constante time-delta so that the game will run
      // determistic and not different on different machines
      if(!game_pause && !Menu::current())
        {
          // Update the world
          check_end_conditions();
          if (end_sequence == ENDSEQUENCE_RUNNING)
             action(frame_ratio/2);
          else if(end_sequence == NO_ENDSEQUENCE)
             action(frame_ratio);
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

      frame_rate.update();

      /* Handle time: */
      if (!time_left.check() && currentsector->player->dying == DYING_NOT
              && !end_sequence)
        currentsector->player->kill(Player::KILL);

      /* Handle music: */
      if(currentsector->player->invincible_timer.check() && !end_sequence)
        {
          currentsector->play_music(HERRING_MUSIC);
        }
      /* are we low on time ? */
      else if (time_left.get_left() < TIME_WARNING && !end_sequence)
        {
          currentsector->play_music(HURRYUP_MUSIC);
        }
      /* or just normal music? */
      else if(currentsector->get_music_type() != LEVEL_MUSIC && !end_sequence)
        {
          currentsector->play_music(LEVEL_MUSIC);
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

void
GameSession::respawn(const std::string& sector, const std::string& spawnpoint)
{
  newsector = sector;
  newspawnpoint = spawnpoint;
}

/* Bounce a brick: */
void bumpbrick(float x, float y)
{
  Sector::current()->add_bouncy_brick(Vector(((int)(x + 1) / 32) * 32,
                         (int)(y / 32) * 32));

  SoundManager::get()->play_sound(IDToSound(SND_BRICK), Vector(x, y), Sector::current()->player->get_pos());
}

/* (Status): */
void
GameSession::drawstatus(DrawingContext& context)
{
  char str[60];
  
  snprintf(str, 60, " %d", global_stats.get_points(SCORE_STAT));
  context.draw_text(white_text, _("SCORE"), Vector(0, 0), LEFT_ALLIGN, LAYER_FOREGROUND1);
  context.draw_text(gold_text, str, Vector(96, 0), LEFT_ALLIGN, LAYER_FOREGROUND1);

  if(st_gl_mode == ST_GL_TEST)
    {
      context.draw_text(white_text, _("Press ESC To Return"), Vector(0,20),
          LEFT_ALLIGN, LAYER_FOREGROUND1);
    }

  if(!time_left.check()) {
    context.draw_text(white_text, _("TIME's UP"), Vector(screen->w/2, 0),
        CENTER_ALLIGN, LAYER_FOREGROUND1);
  } else if (time_left.get_left() > TIME_WARNING || (global_frame_counter % 10) < 5) {
    sprintf(str, " %d", time_left.get_left() / 1000 );
    context.draw_text(white_text, _("TIME"),
        Vector(screen->w/2, 0), CENTER_ALLIGN, LAYER_FOREGROUND1);
    context.draw_text(gold_text, str,
        Vector(screen->w/2 + 4*16, 0), CENTER_ALLIGN, LAYER_FOREGROUND1);
  }

  sprintf(str, " %d", player_status.distros);
  context.draw_text(white_text, _("COINS"),
      Vector(screen->w - white_text->get_text_width(_("COINS"))-white_text->get_text_width("   99"), 0),
        LEFT_ALLIGN, LAYER_FOREGROUND1);
  context.draw_text(gold_text, str,
      Vector(screen->w - gold_text->get_text_width(" 99"), 0),LEFT_ALLIGN, LAYER_FOREGROUND1);

  if (player_status.lives >= 5)
    {
      sprintf(str, "%dx", player_status.lives);
      float x = screen->w - gold_text->get_text_width(str) - tux_life->w;
      context.draw_text(gold_text, str, Vector(x, 20), LEFT_ALLIGN, LAYER_FOREGROUND1);
      context.draw_surface(tux_life, Vector(screen->w - 16, 20),
          LAYER_FOREGROUND1);
    }
  else
    {
      for(int i= 0; i < player_status.lives; ++i)
        context.draw_surface(tux_life, 
            Vector(screen->w - tux_life->w*4 +(tux_life->w*i), 20),
            LAYER_FOREGROUND1);
    }

  context.draw_text(white_text, _("LIVES"),
      Vector(screen->w - white_text->get_text_width(_("LIVES")) - white_text->get_text_width("   99"), 20),
      LEFT_ALLIGN, LAYER_FOREGROUND1);

  if(show_fps)
    {
      sprintf(str, "%2.1f", fps_fps);
      context.draw_text(white_text, "FPS", 
          Vector(screen->w - white_text->get_text_width("FPS     "), 40),
          LEFT_ALLIGN, LAYER_FOREGROUND1);
      context.draw_text(gold_text, str,
          Vector(screen->w-4*16, 40), LEFT_ALLIGN, LAYER_FOREGROUND1);
    }
}

void
GameSession::drawresultscreen(void)
{
  char str[80];

  DrawingContext context;
  currentsector->background->draw(context);  

  context.draw_text(blue_text, _("Result:"), Vector(screen->w/2, 200),
      CENTER_ALLIGN, LAYER_FOREGROUND1);

  sprintf(str, _("SCORE: %d"), global_stats.get_points(SCORE_STAT));
  context.draw_text(gold_text, str, Vector(screen->w/2, 224), CENTER_ALLIGN, LAYER_FOREGROUND1);

  sprintf(str, _("COINS: %d"), player_status.distros);
  context.draw_text(gold_text, str, Vector(screen->w/2, 256), CENTER_ALLIGN, LAYER_FOREGROUND1);

  context.do_drawing();
  
  SDL_Event event;
  wait_for_event(event,2000,5000,true);
}

std::string slotinfo(int slot)
{
  std::string tmp;
  std::string slotfile;
  std::string title;
  std::stringstream stream;
  stream << slot;
  slotfile = st_save_dir + "/slot" + stream.str() + ".stsg";

  lisp_object_t* savegame = lisp_read_from_file(slotfile.c_str());
  if (savegame)
    {
      LispReader reader(lisp_cdr(savegame));
      reader.read_string("title", title);
      lisp_free(savegame);
    }

  if (access(slotfile.c_str(), F_OK) == 0)
    {
      if (!title.empty())
        tmp = "Slot " + stream.str() + " - " + title;
      else
        tmp = "Slot " + stream.str() + " - Savegame";
    }
  else
    tmp = std::string(_("Slot")) + " " + stream.str() + " - " + std::string(_("Free"));

  return tmp;
}

bool process_load_game_menu()
{
  int slot = load_game_menu->check();

  if(slot != -1 && load_game_menu->get_item_by_id(slot).kind == MN_ACTION)
    {
      std::stringstream stream;
      stream << slot;
      std::string slotfile = st_save_dir + "/slot" + stream.str() + ".stsg";

      if (access(slotfile.c_str(), F_OK) != 0)
        {
          draw_intro();
        }

      // shrink_fade(Point((screen->w/2),(screen->h/2)), 1000);
      fadeout(256);

      DrawingContext context;
      context.draw_text(white_text, "Loading...",
                        Vector(screen->w/2, screen->h/2), CENTER_ALLIGN, LAYER_FOREGROUND1);
      context.do_drawing();

      WorldMapNS::WorldMap worldmap;

      // Load the game or at least set the savegame_file variable
      worldmap.loadgame(slotfile);

      worldmap.display();

      Menu::set_current(main_menu);

      Ticks::pause_stop();
      return true;
    }
  else
    {
      return false;
    }
}
