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

#include "game_session.hpp"
#include "msg.hpp"
#include "video/screen.hpp"
#include "audio/sound_manager.hpp"
#include "gui/menu.hpp"
#include "sector.hpp"
#include "level.hpp"
#include "tile.hpp"
#include "player_status.hpp"
#include "object/particlesystem.hpp"
#include "object/background.hpp"
#include "object/tilemap.hpp"
#include "object/camera.hpp"
#include "object/player.hpp"
#include "object/level_time.hpp"
#include "lisp/lisp.hpp"
#include "lisp/parser.hpp"
#include "resources.hpp"
#include "worldmap.hpp"
#include "misc.hpp"
#include "statistics.hpp"
#include "timer.hpp"
#include "object/fireworks.hpp"
#include "textscroller.hpp"
#include "control/codecontroller.hpp"
#include "control/joystickkeyboardcontroller.hpp"
#include "main.hpp"
#include "file_system.hpp"
#include "gameconfig.hpp"
#include "gettext.hpp"
#include "exceptions.hpp"
#include "flip_level_transformer.hpp"

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
  currentsector = 0;
  
  game_pause = false;
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
  
  // get time
  int time = 0;
  for(std::vector<Sector*>::iterator i = level->sectors.begin(); i != level->sectors.end(); ++i)
  {
    Sector* sec = *i;

    for(std::vector<GameObject*>::iterator j = sec->gameobjects.begin();
        j != sec->gameobjects.end(); ++j)
    {
      GameObject* obj = *j;
      
      LevelTime* lt = dynamic_cast<LevelTime*> (obj);
      if(lt)
        time += int(lt->get_level_time());
    }
  }
  global_stats.set_total_points(TIME_NEEDED_STAT, (time == 0) ? -1 : time);

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

  currentsector->play_music(LEVEL_MUSIC);

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

  current_ = NULL;
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
  char str[60];

  sound_manager->stop_music();

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

  sprintf(str, "TUX x %d", player_status->lives);
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
      throw graceful_shutdown();
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
    player_status->lives++;
  }
  if(main_controller->check_cheatcode("lifedown")) {
    player_status->lives--;
  }
  if(main_controller->check_cheatcode("grease")) {
    tux.physic.set_velocity_x(tux.physic.get_velocity_x()*3);
  }
  if(main_controller->check_cheatcode("invincible")) {
    // be invincle for the rest of the level
    tux.invincible_timer.start(10000);
  }
  if(main_controller->check_cheatcode("mortal")) {
    // give up invincibility
    tux.invincible_timer.stop();
  }
  if(main_controller->check_cheatcode("shrink")) {
    // remove powerups
    tux.kill(tux.SHRINK);
  }
  if(main_controller->check_cheatcode("kill")) {
    // kill Tux, but without losing a life
    player_status->lives++;
    tux.kill(tux.KILL);
  }
  if(main_controller->check_cheatcode("whereami")) {
    msg_info("You are at x " << tux.get_pos().x << ", y " << tux.get_pos().y);
  }
#if 0
  if(main_controller->check_cheatcode("grid")) {
    // toggle debug grid
    debug_grid = !debug_grid;
  }
#endif
  if(main_controller->check_cheatcode("gotoend")) {
    // goes to the end of the level
    tux.move(Vector(
          (currentsector->solids->get_width()*32) - (SCREEN_WIDTH*2), 0));
    currentsector->camera->reset(
        Vector(tux.get_pos().x, tux.get_pos().y));
  }
  if(main_controller->check_cheatcode("flip")) {
  	FlipLevelTransformer flip_transformer;
    flip_transformer.transform(GameSession::current()->get_current_level());
  }
  if(main_controller->check_cheatcode("finish")) {
    // finish current sector
    exit_status = ES_LEVEL_FINISHED;
    // don't add points to stats though...
  }
  if(main_controller->check_cheatcode("camera")) {
    msg_info("Camera is at " 
              << Sector::current()->camera->get_translation().x << "," 
              << Sector::current()->camera->get_translation().y);
  }
}

void
GameSession::check_end_conditions()
{
  Player* tux = currentsector->player;

  /* End of level? */
  if(end_sequence && endsequence_timer.check()) {
    exit_status = ES_LEVEL_FINISHED;
    
    // add time spent to statistics
    int tottime = 0, remtime = 0;
    for(std::vector<Sector*>::iterator i = level->sectors.begin(); i != level->sectors.end(); ++i)
    {
      Sector* sec = *i;

      for(std::vector<GameObject*>::iterator j = sec->gameobjects.begin();
          j != sec->gameobjects.end(); ++j)
      {
        GameObject* obj = *j;

        LevelTime* lt = dynamic_cast<LevelTime*> (obj);
        if(lt)
        {
          tottime += int(lt->get_level_time());
          remtime += int(lt->get_remaining_time());
        }
      }
    }
    global_stats.set_points(TIME_NEEDED_STAT, (tottime == 0 ? -1 : (tottime-remtime)));

    return;
  } else if (!end_sequence && tux->is_dead()) {
    if (player_status->lives < 0) { // No more lives!?
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
      msg_warning("Sector '" << newsector << "' not found");
    }
    sector->activate(newspawnpoint);
    sector->play_music(LEVEL_MUSIC);
    currentsector = sector;
    newsector = "";
    newspawnpoint = "";
  }

  // update sounds
  sound_manager->set_listener_position(currentsector->player->get_pos());
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
  context->draw_filled_rect(
      Vector(0,0), Vector(SCREEN_WIDTH, SCREEN_HEIGHT),
      Color(.2, .2, .2, .5), LAYER_FOREGROUND1);
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
      game_time += elapsed_time;

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

    // update sounds
    sound_manager->update();

    /* Time stops in pause mode */
    if(game_pause || Menu::current())
    {
      continue;
    }

    //frame_rate.update();
    
    /* Handle music: */
    if (currentsector->player->invincible_timer.started() && 
            currentsector->player->invincible_timer.get_timeleft() 
            > TUX_INVINCIBLE_TIME_WARNING && !end_sequence)
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
        throw graceful_shutdown();
    }

    if(main_controller->pressed(Controller::JUMP)
        || main_controller->pressed(Controller::ACTION)
        || main_controller->pressed(Controller::PAUSE_MENU)
        || main_controller->pressed(Controller::MENU_SELECT))
      running = false;
    else if(main_controller->pressed(Controller::DOWN))
      box->scrolldown();
    else if(main_controller->pressed(Controller::UP))
      box->scrollup();
    box->draw(*context);
    draw();
    sound_manager->update();
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
    endsequence_timer.start(7.3);
    last_x_pos = -1;
    sound_manager->play_music("music/leveldone.ogg", false);
    currentsector->player->invincible_timer.start(7.3);

    // Stop all clocks.
    for(std::vector<GameObject*>::iterator i = currentsector->gameobjects.begin();
        i != currentsector->gameobjects.end(); ++i)
    {
      GameObject* obj = *i;

      LevelTime* lt = dynamic_cast<LevelTime*> (obj);
      if(lt)
        lt->stop();
    }

    if(sequencename == "fireworks") {
      currentsector->add_object(new Fireworks());
    }
  } else if(sequencename == "stoptux") {
    if(!end_sequence) {
      msg_warning("Final target reached without "
        << "an active end sequence");
      this->start_sequence("endsequence");
    }
    end_sequence =  ENDSEQUENCE_WAITING;
  } else {
    msg_warning("Unknown sequence '" << sequencename << "'");
  }
}

/* (Status): */
void
GameSession::drawstatus(DrawingContext& context)
{
  player_status->draw(context);

  if(config->show_fps) {
    char str[60];
    snprintf(str, sizeof(str), "%3.1f", fps_fps);
    context.draw_text(white_text, "FPS", 
                      Vector(SCREEN_WIDTH -
                             white_text->get_text_width("FPS     ") - BORDER_X, BORDER_Y + 40),
                      LEFT_ALLIGN, LAYER_FOREGROUND1);
    context.draw_text(gold_text, str,
                      Vector(SCREEN_WIDTH-4*16 - BORDER_X, BORDER_Y + 40),
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

//  sprintf(str, _("SCORE: %d"), global_stats.get_points(SCORE_STAT));
//  context.draw_text(gold_text, str, Vector(SCREEN_WIDTH/2, 224), CENTER_ALLIGN, LAYER_FOREGROUND1);

  // y == 256 before removal of score
  sprintf(str, _("COINS: %d"), player_status->coins);
  context.draw_text(gold_text, str, Vector(SCREEN_WIDTH/2, 224), CENTER_ALLIGN, LAYER_FOREGROUND1);

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
  slotfile = "save/slot" + stream.str() + ".stsg";

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
  std::string slotfile = "save/slot" + stream.str() + ".stsg";

  sound_manager->stop_music();
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
