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
#include <config.h>

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
#include <stdexcept>

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
#include "gui/menu.h"
#include "sector.h"
#include "level.h"
#include "scene.h"
#include "tile.h"
#include "object/particlesystem.h"
#include "object/background.h"
#include "object/tilemap.h"
#include "object/camera.h"
#include "object/player.h"
#include "lisp/lisp.h"
#include "lisp/parser.h"
#include "resources.h"
#include "app/gettext.h"
#include "worldmap.h"
#include "intro.h"
#include "misc.h"
#include "statistics.h"
#include "timer.h"
#include "object/fireworks.h"

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

GameSession::GameSession(const std::string& levelfile_, int mode,
    Statistics* statistics)
  : level(0), currentsector(0), st_gl_mode(mode),
    end_sequence(NO_ENDSEQUENCE), levelfile(levelfile_),
    best_level_statistics(statistics)
{
  current_ = this;
  
  game_pause = false;
  fps_fps = 0;

  context = new DrawingContext();

  last_swap_point = Vector(-1, -1);
  last_swap_stats.reset();

  restart_level();
}

void
GameSession::restart_level()
{
  game_pause   = false;
  exit_status  = ES_NONE;
  end_sequence = NO_ENDSEQUENCE;

  last_keys.clear();

#if 0
  Vector tux_pos = Vector(-1,-1);
  if (currentsector)
    { 
      // Tux has lost a life, so we try to respawn him at the nearest reset point
      tux_pos = currentsector->player->base;
    }
#endif
  
  delete level;
  currentsector = 0;

  level = new Level;
  level->load(levelfile);

  global_stats.reset();
  global_stats.set_total_points(COINS_COLLECTED_STAT, level->get_total_coins());
  global_stats.set_total_points(BADGUYS_KILLED_STAT, level->get_total_badguys());
  global_stats.set_total_points(TIME_NEEDED_STAT, level->timelimit);

  currentsector = level->get_sector("main");
  if(!currentsector)
    Termination::abort("Level has no main sector.", "");
  currentsector->activate("main");

#if 0
  // Set Tux to the nearest reset point
  if(tux_pos.x != -1)
    {
    tux_pos = currentsector->get_best_spawn_point(tux_pos);

    if(last_swap_point.x > tux_pos.x)
      tux_pos = last_swap_point;
    else  // new swap point
      {
      last_swap_point = tux_pos;

      last_swap_stats += global_stats;
      }

    currentsector->player->base.x = tux_pos.x;
    currentsector->player->base.y = tux_pos.y;

    // has to reset camera on swapping
    currentsector->camera->reset(Vector(currentsector->player->base.x,
                                        currentsector->player->base.y));
    }
#endif

  if (st_gl_mode != ST_GL_DEMO_GAME)
    {
      if(st_gl_mode == ST_GL_PLAY || st_gl_mode == ST_GL_LOAD_LEVEL_FILE)
        levelintro();
    }

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
  for(Sector::GameObjects::iterator i = currentsector->gameobjects.begin();
      i != currentsector->gameobjects.end(); ++i) {
    Background* background = dynamic_cast<Background*> (*i);
    if(background) {
      background->draw(context);
    }
  }

//  context.draw_text(gold_text, level->get_name(), Vector(screen->w/2, 160),
//      CENTER_ALLIGN, LAYER_FOREGROUND1);
  context.draw_center_text(gold_text, level->get_name(), Vector(0, 160),
      LAYER_FOREGROUND1);

  sprintf(str, "TUX x %d", player_status.lives);
  context.draw_text(white_text, str, Vector(screen->w/2, 210),
      CENTER_ALLIGN, LAYER_FOREGROUND1);

  if((level->get_author().size()) && (level->get_author() != "SuperTux Team"))
    //TODO make author check case/blank-insensitive
    context.draw_text(white_small_text,
      std::string(_("contributed by ")) + level->get_author(), 
      Vector(screen->w/2, 350), CENTER_ALLIGN, LAYER_FOREGROUND1);


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
  time_left.start(level->timelimit);
  Ticks::pause_init();
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
      tux.key_event((SDLKey)keymap.up, UP);
      tux.key_event((SDLKey)keymap.down, UP);
      tux.key_event((SDLKey)keymap.left, UP);
      tux.key_event((SDLKey)keymap.right, UP);
      tux.key_event((SDLKey)keymap.jump, UP);
      tux.key_event((SDLKey)keymap.power, UP);

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

      if (int(last_x_pos) == int(tux.get_pos().x))
        tux.input.up    = DOWN; 
      else
        tux.input.up    = UP; 

      last_x_pos = tux.get_pos().x;

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
                              tux.get_pos().x, tux.get_pos().y);
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
                        if(compare_last(last_keys, "grease"))
                          {
                          tux.physic.set_velocity_x(tux.physic.get_velocity_x()*3);
                          last_keys.clear();
                          }
                        if(compare_last(last_keys, "invincible"))
                          {    // be invincle for the rest of the level
                          tux.invincible_timer.start(10000);
                          last_keys.clear();
                          }
                        if(compare_last(last_keys, "shrink"))
                          {    // remove powerups
                          tux.kill(tux.SHRINK);
                          last_keys.clear();
                          }
                        if(compare_last(last_keys, "kill"))
                          {    // kill Tux, but without losing a life
                          player_status.lives++;
                          tux.kill(tux.KILL);
                          last_keys.clear();
                          }
                        if(compare_last(last_keys, "grid"))
                          {    // toggle debug grid
                          debug_grid = !debug_grid;
                          last_keys.clear();
                          }
                        if(compare_last(last_keys, "hover"))
                          {    // toggle hover ability on/off
                          tux.enable_hover = !tux.enable_hover;
                          last_keys.clear();
                          }
                        if(compare_last(last_keys, "gotoend"))
                          {    // goes to the end of the level
                          tux.move(Vector(
                              (currentsector->solids->get_width()*32) 
                                - (screen->w*2),
                                0));
                          currentsector->camera->reset(
                              Vector(tux.get_pos().x, tux.get_pos().y));
                          last_keys.clear();
                          }
                        // temporary to help player's choosing a flapping
                        if(compare_last(last_keys, "marek"))
                          {
                          tux.flapping_mode = Player::MAREK_FLAP;
                          last_keys.clear();
                          }
                        if(compare_last(last_keys, "ricardo"))
                          {
                          tux.flapping_mode = Player::RICARDO_FLAP;
                          last_keys.clear();
                          }
                        if(compare_last(last_keys, "ryan"))
                          {
                          tux.flapping_mode = Player::RYAN_FLAP;
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
                        {
                        tux.input.up = DOWN;
                        tux.input.down = UP;
                        }
                      else if (event.jaxis.value < -joystick_keymap.dead_zone)
                        {
                        tux.input.up = UP;
                        tux.input.down = DOWN;
                        }
                      else
                        {
                        tux.input.up = DOWN;
                        tux.input.down = DOWN;
                        }
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
                    tux.input.jump = DOWN;
                  else if (event.jbutton.button == joystick_keymap.b_button)
                    tux.input.fire = DOWN;
                  else if (event.jbutton.button == joystick_keymap.start_button)
                    on_escape_press();
                  break;
                case SDL_JOYBUTTONUP:
                  if (event.jbutton.button == joystick_keymap.a_button)
                    tux.input.jump = UP;
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
  if(end_sequence && endsequence_timer.check()) {
      exit_status = ES_LEVEL_FINISHED;
      global_stats += last_swap_stats;  // add swap points stats
      return;
  } else if (!end_sequence && tux->is_dead()) {
    player_status.bonus = PlayerStatus::NO_BONUS;

    if (player_status.lives < 0) { // No more lives!?
      exit_status = ES_GAME_OVER;
    } else { // Still has lives, so reset Tux to the levelstart
      restart_level();
    }
    
    return;
  }
}

void
GameSession::action(float elapsed_time)
{
  // advance timers
  if (exit_status == ES_NONE && !currentsector->player->growing_timer.check())
    {
      // Update Tux and the World
      currentsector->action(elapsed_time);
    }

  // respawning in new sector?
  if(newsector != "" && newspawnpoint != "") {
    Sector* sector = level->get_sector(newsector);
    currentsector = sector;
    currentsector->activate(newspawnpoint);
    currentsector->play_music(LEVEL_MUSIC);
    newsector = newspawnpoint = "";
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

  // Eat unneeded events
  SDL_Event event;
  while(SDL_PollEvent(&event))
  {}

  draw();

  Uint32 lastticks = SDL_GetTicks();
  fps_ticks = SDL_GetTicks();

  while (exit_status == ES_NONE) {
    Uint32 ticks = SDL_GetTicks();
    float elapsed_time = float(ticks - lastticks) / 1000.;
    if(!game_pause)
      global_time += elapsed_time;
    lastticks = ticks;

    // 40fps is minimum
    if(elapsed_time > .025)
      elapsed_time = .025;
    
    /* Handle events: */
    currentsector->player->input.old_fire = currentsector->player->input.fire;
    currentsector->player->input.old_up = currentsector->player->input.old_up;

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
        action(elapsed_time/2);
      else if(end_sequence == NO_ENDSEQUENCE)
        action(elapsed_time);
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

    //frame_rate.update();
    
    /* Handle time: */
    if (time_left.check() && currentsector->player->dying == DYING_NOT
        && !end_sequence)
      currentsector->player->kill(Player::KILL);
    
    /* Handle music: */
    if(currentsector->player->invincible_timer.started() && !end_sequence)
    {
      currentsector->play_music(HERRING_MUSIC);
    }
    /* are we low on time ? */
    else if (time_left.get_timeleft() < TIME_WARNING && !end_sequence)
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
      
      if(SDL_GetTicks() - fps_ticks >= 500)
      {
        fps_fps = (float) fps_cnt / .5;
        fps_cnt = 0;
        fps_ticks = SDL_GetTicks();
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

void
GameSession::start_sequence(const std::string& sequencename)
{
  if(sequencename == "endsequence") {
    if(end_sequence)
      return;
    
    end_sequence = ENDSEQUENCE_RUNNING;
    endsequence_timer.start(7.0); // 7 seconds until we finish the map
    last_x_pos = -1;
    SoundManager::get()->play_music(level_end_song, 0);
    currentsector->player->invincible_timer.start(7.0);

    // add left time to stats
    global_stats.set_points(TIME_NEEDED_STAT,
        int(time_left.get_period() - time_left.get_timeleft()));

    if(level->get_end_sequence_type() == Level::FIREWORKS_ENDSEQ_ANIM) {
      currentsector->add_object(new Fireworks());
    }
  } else {
    std::cout << "Unknown sequence '" << sequencename << "'.\n";
  }
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

  if(time_left.check()) {
    context.draw_text(white_text, _("TIME's UP"), Vector(screen->w/2, 0),
        CENTER_ALLIGN, LAYER_FOREGROUND1);
  } else if (time_left.get_timeleft() > TIME_WARNING
      || int(global_time * 2.5) % 2) {
    sprintf(str, " %d", int(time_left.get_timeleft()));
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
GameSession::drawresultscreen()
{
  char str[80];

  DrawingContext context;
  for(Sector::GameObjects::iterator i = currentsector->gameobjects.begin();
      i != currentsector->gameobjects.end(); ++i) {
    Background* background = dynamic_cast<Background*> (*i);
    if(background) {
      background->draw(context);
    }
  }

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

  try {
    lisp::Parser parser;
    std::auto_ptr<lisp::Lisp> root (parser.parse(slotfile));

    const lisp::Lisp* savegame = root->get_lisp("supertux-savegame");
    if(!savegame)
      throw std::runtime_error("file is not a supertux-savegame.");

    savegame->get("title", title);
  } catch(std::exception& e) {
    return std::string(_("Slot")) + " " + stream.str() + " - " +
      std::string(_("Free"));
  }

  return std::string("Slot ") + stream.str() + " - " + title;
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
          shrink_fade(Vector(screen->w/2,screen->h/2), 600);
          draw_intro();
        }

      fadeout(256);
      DrawingContext context;
      context.draw_text(white_text, "Loading...",
                        Vector(screen->w/2, screen->h/2), CENTER_ALLIGN, LAYER_FOREGROUND1);
      context.do_drawing();

      WorldMapNS::WorldMap worldmap;

      worldmap.set_map_filename("icyisland.stwm");
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
