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
#include <fstream>
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

#include <SDL.h>

#ifndef WIN32
#include <sys/types.h>
#include <ctype.h>
#endif

#include "game_session.h"
#include "video/screen.h"
#include "gui/menu.h"
#include "sector.h"
#include "level.h"
#include "tile.h"
#include "player_status.h"
#include "object/particlesystem.h"
#include "object/background.h"
#include "object/tilemap.h"
#include "object/camera.h"
#include "object/player.h"
#include "lisp/lisp.h"
#include "lisp/parser.h"
#include "resources.h"
#include "worldmap.h"
#include "misc.h"
#include "statistics.h"
#include "timer.h"
#include "object/fireworks.h"
#include "textscroller.h"
#include "control/codecontroller.h"
#include "control/joystickkeyboardcontroller.h"
#include "main.h"
#include "file_system.h"
#include "gameconfig.h"
#include "gettext.h"

// the engine will be run with a logical framerate of 64fps.
// We chose 64fps here because it is a power of 2, so 1/64 gives an "even"
// binary fraction...
static const float LOGICAL_FPS = 64.0;

GameSession* GameSession::current_ = 0;

GameSession::GameSession(const std::string& levelfile_, GameSessionMode mode,
    Statistics* statistics)
  : level(0), currentsector(0), mode(mode),
    end_sequence(NO_ENDSEQUENCE), end_sequence_controller(0),
    levelfile(levelfile_), best_level_statistics(statistics),
    capture_demo_stream(0), playback_demo_stream(0), demo_controller(0)
{
  current_ = this;
  
  game_pause = false;
  music_playing = false;
  fps_fps = 0;

  context = new DrawingContext();

  restart_level();
}

void
GameSession::restart_level()
{
  game_pause   = false;
  exit_status  = ES_NONE;
  end_sequence = NO_ENDSEQUENCE;

  main_controller->reset();

  delete level;
  currentsector = 0;

  level = new Level;
  level->load(levelfile);

  global_stats.reset();
  global_stats.set_total_points(COINS_COLLECTED_STAT, level->get_total_coins());
  global_stats.set_total_points(BADGUYS_KILLED_STAT, level->get_total_badguys());
  //global_stats.set_total_points(TIME_NEEDED_STAT, level->timelimit);

  if(reset_sector != "") {
    currentsector = level->get_sector(reset_sector);
    if(!currentsector) {
      std::stringstream msg;
      msg << "Couldn't find sector '" << reset_sector << "' for resetting tux.";
      throw std::runtime_error(msg.str());
    }
    currentsector->activate(reset_pos);
  } else {
    currentsector = level->get_sector("main");
    if(!currentsector)
      throw std::runtime_error("Couldn't find main sector");
    currentsector->activate("main");
  }
  
  if(mode == ST_GL_PLAY || mode == ST_GL_LOAD_LEVEL_FILE)
    levelintro();

  if (!music_playing)
  {
    currentsector->play_music(LEVEL_MUSIC);
    music_playing = true;
  }

  if(capture_file != "")
    record_demo(capture_file);
}

GameSession::~GameSession()
{
  delete capture_demo_stream;
  delete playback_demo_stream;
  delete demo_controller;

  delete end_sequence_controller;
  delete level;
  delete context;
}

void
GameSession::record_demo(const std::string& filename)
{
  delete capture_demo_stream;
  
  capture_demo_stream = new std::ofstream(filename.c_str()); 
  if(!capture_demo_stream->good()) {
    std::stringstream msg;
    msg << "Couldn't open demo file '" << filename << "' for writing.";
    throw std::runtime_error(msg.str());
  }
  capture_file = filename;
}

void
GameSession::play_demo(const std::string& filename)
{
  delete playback_demo_stream;
  delete demo_controller;
  
  playback_demo_stream = new std::ifstream(filename.c_str());
  if(!playback_demo_stream->good()) {
    std::stringstream msg;
    msg << "Couldn't open demo file '" << filename << "' for reading.";
    throw std::runtime_error(msg.str());
  }

  Player& tux = *currentsector->player;
  demo_controller = new CodeController();
  tux.set_controller(demo_controller);
}

void
GameSession::levelintro()
{
  //sound_manager->halt_music();
  
  char str[60];

  DrawingContext context;
  for(Sector::GameObjects::iterator i = currentsector->gameobjects.begin();
      i != currentsector->gameobjects.end(); ++i) {
    Background* background = dynamic_cast<Background*> (*i);
    if(background) {
      background->draw(context);
    }
  }

//  context.draw_text(gold_text, level->get_name(), Vector(SCREEN_WIDTH/2, 160),
//      CENTER_ALLIGN, LAYER_FOREGROUND1);
  context.draw_center_text(gold_text, level->get_name(), Vector(0, 160),
      LAYER_FOREGROUND1);

  sprintf(str, "TUX x %d", player_status.lives);
  context.draw_text(white_text, str, Vector(SCREEN_WIDTH/2, 210),
      CENTER_ALLIGN, LAYER_FOREGROUND1);

  if((level->get_author().size()) && (level->get_author() != "SuperTux Team"))
    //TODO make author check case/blank-insensitive
    context.draw_text(white_small_text,
      std::string(_("contributed by ")) + level->get_author(), 
      Vector(SCREEN_WIDTH/2, 350), CENTER_ALLIGN, LAYER_FOREGROUND1);


  if(best_level_statistics != NULL)
    best_level_statistics->draw_message_info(context, _("Best Level Statistics"));

  context.do_drawing();

  wait_for_event(1.0, 3.0);
}

void
GameSession::on_escape_press()
{
  if(currentsector->player->is_dying() || end_sequence != NO_ENDSEQUENCE)
    return;   // don't let the player open the menu, when he is dying
  
  if(mode == ST_GL_TEST) {
    exit_status = ES_LEVEL_ABORT;
  } else if (!Menu::current()) {
    Menu::set_current(game_menu);
    game_menu->set_active_item(MNID_CONTINUE);
    game_pause = true;
  } else {
    game_pause = false;
  }
}

void
GameSession::process_events()
{
  Player& tux = *currentsector->player;
  main_controller->update();

  // end of pause mode?
  if(!Menu::current() && game_pause) {
    game_pause = false;
  }

  if (end_sequence != NO_ENDSEQUENCE) {
    if(end_sequence_controller == 0) {
      end_sequence_controller = new CodeController();
      tux.set_controller(end_sequence_controller);
    }

    end_sequence_controller->press(Controller::RIGHT);
    
    if (int(last_x_pos) == int(tux.get_pos().x))
      end_sequence_controller->press(Controller::JUMP);    
    last_x_pos = tux.get_pos().x;
  }

  main_controller->update();
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    /* Check for menu-events, if the menu is shown */
    if (Menu::current())
      Menu::current()->event(event);
    main_controller->process_event(event);
    if(event.type == SDL_QUIT)
      throw std::runtime_error("Received window close");  
  }

  // playback a demo?
  if(playback_demo_stream != 0) {
    demo_controller->update();
    char left = false;
    char right = false;
    char up = false;
    char down = false;
    char jump = false;
    char action = false;
    playback_demo_stream->get(left);
    playback_demo_stream->get(right);
    playback_demo_stream->get(up);
    playback_demo_stream->get(down);
    playback_demo_stream->get(jump);
    playback_demo_stream->get(action);
    demo_controller->press(Controller::LEFT, left);
    demo_controller->press(Controller::RIGHT, right);
    demo_controller->press(Controller::UP, up);
    demo_controller->press(Controller::DOWN, down);
    demo_controller->press(Controller::JUMP, jump);
    demo_controller->press(Controller::ACTION, action);
  }

  // save input for demo?
  if(capture_demo_stream != 0) {
    capture_demo_stream ->put(main_controller->hold(Controller::LEFT));
    capture_demo_stream ->put(main_controller->hold(Controller::RIGHT));
    capture_demo_stream ->put(main_controller->hold(Controller::UP));
    capture_demo_stream ->put(main_controller->hold(Controller::DOWN));
    capture_demo_stream ->put(main_controller->hold(Controller::JUMP));   
    capture_demo_stream ->put(main_controller->hold(Controller::ACTION));
  }
}

void
GameSession::try_cheats()
{
  if(currentsector == 0)
    return;
  Player& tux = *currentsector->player;
  
  // Cheating words (the goal of this is really for debugging,
  // but could be used for some cheating, nothing wrong with that)
  if(main_controller->check_cheatcode("grow")) {
    tux.set_bonus(GROWUP_BONUS, false);
  }
  if(main_controller->check_cheatcode("fire")) {
    tux.set_bonus(FIRE_BONUS, false);
  }
  if(main_controller->check_cheatcode("ice")) {
    tux.set_bonus(ICE_BONUS, false);
  }
  if(main_controller->check_cheatcode("lifeup")) {
    player_status.lives++;
  }
  if(main_controller->check_cheatcode("lifedown")) {
    player_status.lives--;
  }
  if(main_controller->check_cheatcode("grease")) {
    tux.physic.set_velocity_x(tux.physic.get_velocity_x()*3);
  }
  if(main_controller->check_cheatcode("invincible")) {
    // be invincle for the rest of the level
    tux.invincible_timer.start(10000);
  }
  if(main_controller->check_cheatcode("shrink")) {
    // remove powerups
    tux.kill(tux.SHRINK);
  }
  if(main_controller->check_cheatcode("kill")) {
    // kill Tux, but without losing a life
    player_status.lives++;
    tux.kill(tux.KILL);
  }
#if 0
  if(main_controller->check_cheatcode("grid")) {
    // toggle debug grid
    debug_grid = !debug_grid;
  }
#endif
  if(main_controller->check_cheatcode("hover")) {
    // toggle hover ability on/off
    tux.enable_hover = !tux.enable_hover;
  }
  if(main_controller->check_cheatcode("gotoend")) {
    // goes to the end of the level
    tux.move(Vector(
          (currentsector->solids->get_width()*32) - (SCREEN_WIDTH*2), 0));
    currentsector->camera->reset(
        Vector(tux.get_pos().x, tux.get_pos().y));
  }
  if(main_controller->check_cheatcode("finish")) {
    // finish current sector
    exit_status = ES_LEVEL_FINISHED;
    // don't add points to stats though...
  }
  // temporary to help player's choosing a flapping
  if(main_controller->check_cheatcode("marek")) {
    tux.flapping_mode = Player::MAREK_FLAP;
  }
  if(main_controller->check_cheatcode("ricardo")) {
    tux.flapping_mode = Player::RICARDO_FLAP;
  }
  if(main_controller->check_cheatcode("ryan")) {
    tux.flapping_mode = Player::RYAN_FLAP;
  }
}

void
GameSession::check_end_conditions()
{
  Player* tux = currentsector->player;

  /* End of level? */
  if(end_sequence && endsequence_timer.check()) {
    exit_status = ES_LEVEL_FINISHED;
    return;
  } else if (!end_sequence && tux->is_dead()) {
    if (player_status.lives < 0) { // No more lives!?
      exit_status = ES_GAME_OVER;
    } else { // Still has lives, so reset Tux to the levelstart
      restart_level();
    }
    
    return;
  }
}

void
GameSession::update(float elapsed_time)
{
  // handle controller
  if(main_controller->pressed(Controller::PAUSE_MENU))
    on_escape_press();
  
  // advance timers
  if(!currentsector->player->growing_timer.started()) {
    // Update Tux and the World
    currentsector->update(elapsed_time);
  }

  // respawning in new sector?
  if(newsector != "" && newspawnpoint != "") {
    Sector* sector = level->get_sector(newsector);
    if(sector == 0) {
      std::cerr << "Sector '" << newsector << "' not found.\n";
    }
    sector->activate(newspawnpoint);
    sector->play_music(LEVEL_MUSIC);
    currentsector = sector;
    newsector = "";
    newspawnpoint = "";
  }
}

void 
GameSession::draw()
{
  currentsector->draw(*context);
  drawstatus(*context);

  if(game_pause)
    draw_pause();

  if(Menu::current()) {
    Menu::current()->draw(*context);
  }

  context->do_drawing();
}

void
GameSession::draw_pause()
{
  int x = SCREEN_HEIGHT / 20;
  for(int i = 0; i < x; ++i) {
    context->draw_filled_rect(
        Vector(i % 2 ? (pause_menu_frame * i)%SCREEN_WIDTH :
          -((pause_menu_frame * i)%SCREEN_WIDTH)
          ,(i*20+pause_menu_frame)%SCREEN_HEIGHT),
        Vector(SCREEN_WIDTH,10),
        Color(20,20,20, rand() % 20 + 1), LAYER_FOREGROUND1+1);
  }

  context->draw_filled_rect(
      Vector(0,0), Vector(SCREEN_WIDTH, SCREEN_HEIGHT),
      Color(rand() % 50, rand() % 50, rand() % 50, 128), LAYER_FOREGROUND1);
#if 0
  context->draw_text(blue_text, _("PAUSE - Press 'P' To Play"),
      Vector(SCREEN_WIDTH/2, 230), CENTER_ALLIGN, LAYER_FOREGROUND1+2);

  const char* str1 = _("Playing: ");
  const char* str2 = level->get_name().c_str();

  context->draw_text(blue_text, str1,
      Vector((SCREEN_WIDTH - (blue_text->get_text_width(str1) + white_text->get_text_width(str2)))/2, 340),
      LEFT_ALLIGN, LAYER_FOREGROUND1+2);
  context->draw_text(white_text, str2,
      Vector(((SCREEN_WIDTH - (blue_text->get_text_width(str1) + white_text->get_text_width(str2)))/2)+blue_text->get_text_width(str1), 340),
      LEFT_ALLIGN, LAYER_FOREGROUND1+2);
#endif
}
  
void
GameSession::process_menu()
{
  Menu* menu = Menu::current();
  if(menu) {
    menu->update();

    if(menu == game_menu) {
      switch (game_menu->check()) {
        case MNID_CONTINUE:
          Menu::set_current(0);
          break;
        case MNID_ABORTLEVEL:
          Menu::set_current(0);
          exit_status = ES_LEVEL_ABORT;
          break;
      }
    } else if(menu == options_menu) {
      process_options_menu();
    } else if(menu == load_game_menu ) {
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

  Uint32 fps_ticks = SDL_GetTicks();
  Uint32 fps_nextframe_ticks = SDL_GetTicks();
  Uint32 ticks;
  bool skipdraw = false;

  while (exit_status == ES_NONE) {
    // we run in a logical framerate so elapsed time is a constant
    static const float elapsed_time = 1.0 / LOGICAL_FPS;
    // old code... float elapsed_time = float(ticks - lastticks) / 1000.;
    if(!game_pause)
      global_time += elapsed_time;

    // regulate fps
    ticks = SDL_GetTicks();
    if(ticks > fps_nextframe_ticks) {
      if(skipdraw == true) {
        // already skipped last frame? we have to slow down the game then...
        skipdraw = false;
        fps_nextframe_ticks -= (Uint32) (1000.0 / LOGICAL_FPS);
      } else {
        // don't draw all frames when we're getting too slow
        skipdraw = true;
      }
    } else {
      skipdraw = false;
      while(fps_nextframe_ticks > ticks) {
        /* just wait */
        // If we really have to wait long, then do an imprecise SDL_Delay()
        Uint32 diff = fps_nextframe_ticks - ticks;
        if(diff > 15) {
          SDL_Delay(diff - 10);
        } 
        ticks = SDL_GetTicks();
      }
    }
    fps_nextframe_ticks = ticks + (Uint32) (1000.0 / LOGICAL_FPS);

#if 0
    float diff = SDL_GetTicks() - fps_nextframe_ticks;
    if (diff > 5.0) {
         // sets the ticks that must have elapsed
    	fps_nextframe_ticks = SDL_GetTicks() + (1000.0 / LOGICAL_FPS);
    } else {
        // sets the ticks that must have elapsed
        // in order for the next frame to start.
    	fps_nextframe_ticks += 1000.0 / LOGICAL_FPS;
    }
#endif

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
        update(elapsed_time/2);
      else if(end_sequence == NO_ENDSEQUENCE)
        update(elapsed_time);
    }
    else
    {
      ++pause_menu_frame;
    }

    if(!skipdraw)
      draw();

    /* Time stops in pause mode */
    if(game_pause || Menu::current())
    {
      continue;
    }

    //frame_rate.update();
    
    /* Handle music: */
    if (currentsector->player->invincible_timer.started() && !end_sequence)
    {
      currentsector->play_music(HERRING_MUSIC);
    }
    /* or just normal music? */
    else if(currentsector->get_music_type() != LEVEL_MUSIC && !end_sequence)
    {
      currentsector->play_music(LEVEL_MUSIC);
    }

    /* Calculate frames per second */
    if(config->show_fps)
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
 
  // just in case
  currentsector = 0;
  main_controller->reset();
  return exit_status;
}

void
GameSession::finish(bool win)
{
  if(win)
    exit_status = ES_LEVEL_FINISHED;
  else
    exit_status = ES_LEVEL_ABORT;
}

void
GameSession::respawn(const std::string& sector, const std::string& spawnpoint)
{
  newsector = sector;
  newspawnpoint = spawnpoint;
}

void
GameSession::set_reset_point(const std::string& sector, const Vector& pos)
{
  reset_sector = sector;
  reset_pos = pos;
}

std::string
GameSession::get_working_directory()
{
  return FileSystem::dirname(levelfile);
}

void
GameSession::display_info_box(const std::string& text)
{
  InfoBox* box = new InfoBox(text);

  bool running = true;
  while(running)  {

    main_controller->update();
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      main_controller->process_event(event);
      if(event.type == SDL_QUIT)
        throw std::runtime_error("Received window close event");
    }

    if(main_controller->pressed(Controller::JUMP)
        || main_controller->pressed(Controller::ACTION)
        || main_controller->pressed(Controller::PAUSE_MENU)
        || main_controller->pressed(Controller::MENU_SELECT))
      running = false;
    box->draw(*context);
    draw();
  }

  delete box;
}

void
GameSession::start_sequence(const std::string& sequencename)
{
  if(sequencename == "endsequence" || sequencename == "fireworks") {
    if(end_sequence)
      return;
    
    end_sequence = ENDSEQUENCE_RUNNING;
    endsequence_timer.start(7.0); // 7 seconds until we finish the map
    last_x_pos = -1;
    sound_manager->play_music(level_end_song, 0);
    currentsector->player->invincible_timer.start(7.0);

    if(sequencename == "fireworks") {
      currentsector->add_object(new Fireworks());
    }
  } else if(sequencename == "stoptux") {
    end_sequence =  ENDSEQUENCE_WAITING;
  } else {
    std::cout << "Unknown sequence '" << sequencename << "'.\n";
  }
}

/* (Status): */
void
GameSession::drawstatus(DrawingContext& context)
{
  player_status.draw(context);

  if(config->show_fps) {
    char str[60];
    snprintf(str, sizeof(str), "%2.1f", fps_fps);
    context.draw_text(white_text, "FPS", 
                      Vector(SCREEN_WIDTH -
                             white_text->get_text_width("FPS     "), 40),
                      LEFT_ALLIGN, LAYER_FOREGROUND1);
    context.draw_text(gold_text, str,
                      Vector(SCREEN_WIDTH-4*16, 40),
                      LEFT_ALLIGN, LAYER_FOREGROUND1);
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

  context.draw_text(blue_text, _("Result:"), Vector(SCREEN_WIDTH/2, 200),
      CENTER_ALLIGN, LAYER_FOREGROUND1);

  sprintf(str, _("SCORE: %d"), global_stats.get_points(SCORE_STAT));
  context.draw_text(gold_text, str, Vector(SCREEN_WIDTH/2, 224), CENTER_ALLIGN, LAYER_FOREGROUND1);

  sprintf(str, _("COINS: %d"), player_status.coins);
  context.draw_text(gold_text, str, Vector(SCREEN_WIDTH/2, 256), CENTER_ALLIGN, LAYER_FOREGROUND1);

  context.do_drawing();
  
  wait_for_event(2.0, 5.0);
}

std::string slotinfo(int slot)
{
  std::string tmp;
  std::string slotfile;
  std::string title;
  std::stringstream stream;
  stream << slot;
  slotfile = user_dir + "/save/slot" + stream.str() + ".stsg";

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

  if(slot == -1)
    return false;
  
  if(load_game_menu->get_item_by_id(slot).kind != MN_ACTION)
    return false;
  
  std::stringstream stream;
  stream << slot;
  std::string slotfile = user_dir + "/save/slot" + stream.str() + ".stsg";

  fadeout(256);
  DrawingContext context;
  context.draw_text(white_text, "Loading...",
                    Vector(SCREEN_WIDTH/2, SCREEN_HEIGHT/2),
                    CENTER_ALLIGN, LAYER_FOREGROUND1);
  context.do_drawing();

  WorldMapNS::WorldMap worldmap;

  worldmap.set_map_filename("/levels/world1/worldmap.stwm");
  // Load the game or at least set the savegame_file variable
  worldmap.loadgame(slotfile);

  worldmap.display();

  Menu::set_current(main_menu);

  return true;
}
