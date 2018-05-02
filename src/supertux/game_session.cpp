//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "supertux/game_session.hpp"

#include <float.h>
#include <fstream>

#include "audio/sound_manager.hpp"
#include "control/input_manager.hpp"
#include "gui/menu.hpp"
#include "gui/menu_manager.hpp"
#include "object/camera.hpp"
#include "object/endsequence_fireworks.hpp"
#include "object/endsequence_walkleft.hpp"
#include "object/endsequence_walkright.hpp"
#include "object/level_time.hpp"
#include "object/player.hpp"
#include "scripting/scripting.hpp"
#include "scripting/squirrel_util.hpp"
#include "supertux/fadein.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "supertux/level_parser.hpp"
#include "supertux/levelintro.hpp"
#include "supertux/levelset_screen.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/menu/options_menu.hpp"
#include "supertux/player_status.hpp"
#include "supertux/savegame.hpp"
#include "supertux/screen_fade.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/sector.hpp"
#include "util/file_system.hpp"
#include "util/gettext.hpp"
#include "worldmap/worldmap.hpp"

#ifdef WIN32
#  define snprintf _snprintf
#endif

GameSession::GameSession(const std::string& levelfile_, Savegame& savegame, Statistics* statistics) :
  GameSessionRecorder(),
  reset_button(false),
  level(),
  old_level(),
  statistics_backdrop(Surface::create("images/engine/menu/score-backdrop.png")),
  scripts(),
  currentsector(nullptr),
  pause_menu_frame(),
  end_sequence(0),
  game_pause(false),
  speed_before_pause(ScreenManager::current()->get_speed()),
  levelfile(levelfile_),
  reset_sector(),
  reset_pos(),
  newsector(),
  newspawnpoint(),
  best_level_statistics(statistics),
  m_savegame(savegame),
  play_time(0),
  edit_mode(false),
  levelintro_shown(false),
  coins_at_start(),
  bonus_at_start(),
  max_fire_bullets_at_start(),
  max_ice_bullets_at_start(),
  active(false),
  end_seq_started(false)
{
  if (restart_level() != 0)
    throw std::runtime_error ("Initializing the level failed.");
}

void
GameSession::reset_level()
{
  currentsector->player->set_bonus(bonus_at_start);
  PlayerStatus *currentStatus = m_savegame.get_player_status();
  currentStatus->coins = coins_at_start;
  currentStatus->max_fire_bullets = max_fire_bullets_at_start;
  currentStatus->max_ice_bullets = max_ice_bullets_at_start;
  reset_sector = "";
  reset_pos = Vector();
}

int
GameSession::restart_level(bool after_death)
{
    PlayerStatus* currentStatus = m_savegame.get_player_status();
    coins_at_start = currentStatus->coins;
    bonus_at_start = currentStatus->bonus;
    max_fire_bullets_at_start = currentStatus->max_fire_bullets;
    max_ice_bullets_at_start = currentStatus->max_ice_bullets;

  if (edit_mode) {
    force_ghost_mode();
    return (-1);
  }

  game_pause   = false;
  end_sequence = 0;

  InputManager::current()->reset();

  currentsector = 0;

  const std::string base_dir = FileSystem::dirname(levelfile);
  if(base_dir == "./") {
    levelfile = FileSystem::basename(levelfile);
  }

  try {
    old_level = std::move(level);
    level = LevelParser::from_file(levelfile);
    level->stats.total_coins = level->get_total_coins();
    level->stats.total_badguys = level->get_total_badguys();
    level->stats.total_secrets = level->get_total_secrets();
    level->stats.reset();

    if(!reset_sector.empty()) {
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
      play_time = 0;
      currentsector->activate("main");
    }
  } catch(std::exception& e) {
    log_fatal << "Couldn't start level: " << e.what() << std::endl;
    ScreenManager::current()->pop_screen();
    return (-1);
  }
  if(after_death == true) {
    currentsector->resume_music();
  }
  else {
    SoundManager::current()->stop_music();
    currentsector->play_music(LEVEL_MUSIC);
  }

  start_recording();

  return (0);
}

GameSession::~GameSession()
{
}

void
GameSession::on_escape_press()
{
  if(currentsector->player->is_dying() || end_sequence)
  {
    // Let the timers run out, we fast-forward them to force past a sequence
    if (end_sequence)
      end_sequence->stop();

    currentsector->player->dying_timer.start(FLT_EPSILON);
    return;   // don't let the player open the menu, when he is dying
  }

  toggle_pause();
}

void
GameSession::toggle_pause()
{
  // pause
  if (!game_pause && !MenuManager::instance().is_active())
  {
    speed_before_pause = ScreenManager::current()->get_speed();
    ScreenManager::current()->set_speed(0);
    MenuManager::instance().set_menu(MenuStorage::GAME_MENU);
    SoundManager::current()->pause_sounds();
    currentsector->stop_looping_sounds();
    SoundManager::current()->pause_music();
    game_pause = true;
  }

  // unpause is done in update() after the menu is processed
}

void
GameSession::abort_level()
{
  MenuManager::instance().clear_menu_stack();
  ScreenManager::current()->pop_screen();
  currentsector->player->set_bonus(bonus_at_start);
  PlayerStatus *currentStatus = m_savegame.get_player_status();
  currentStatus->coins = coins_at_start;
  currentStatus->max_fire_bullets = max_fire_bullets_at_start;
  currentStatus->max_ice_bullets = max_ice_bullets_at_start;
  SoundManager::current()->stop_sounds();
}

bool
GameSession::is_active() const
{
  return !game_pause && active && !end_sequence;
}

void
GameSession::set_editmode(bool edit_mode_)
{
  if (this->edit_mode == edit_mode_) return;
  this->edit_mode = edit_mode_;

  currentsector->get_players()[0]->set_edit_mode(edit_mode_);

  if (edit_mode_) {

    // entering edit mode

  } else {

    // leaving edit mode
    restart_level();

  }
}

void
GameSession::force_ghost_mode()
{
  currentsector->get_players()[0]->set_ghost_mode(true);
}

void
GameSession::check_end_conditions()
{
  Player* tux = currentsector->player;

  /* End of level? */
  if(end_sequence && end_sequence->is_done()) {
    finish(true);
  } else if (!end_sequence && tux->is_dead()) {
    restart_level(true);
  }
}

void
GameSession::draw(DrawingContext& context)
{
  currentsector->draw(context);
  drawstatus(context);

  if(game_pause)
    draw_pause(context);
}


void
GameSession::on_window_resize()
{
  currentsector->on_window_resize();
}

void
GameSession::draw_pause(DrawingContext& context)
{
  context.draw_filled_rect(
    Vector(0,0), Vector(SCREEN_WIDTH, SCREEN_HEIGHT),
    Color(0.0f, 0.0f, 0.0f, .25f), LAYER_FOREGROUND1);
}

void
GameSession::setup()
{
  if (currentsector == NULL)
    return;

  if(currentsector != Sector::current()) {
    currentsector->activate(currentsector->player->get_pos());
  }
  currentsector->play_music(LEVEL_MUSIC);

  int total_stats_to_be_collected = level->stats.total_coins + level->stats.total_badguys + level->stats.total_secrets;
  if ((!levelintro_shown) && (total_stats_to_be_collected > 0)) {
    levelintro_shown = true;
    active = false;
    ScreenManager::current()->push_screen(std::unique_ptr<Screen>(new LevelIntro(level.get(), best_level_statistics, m_savegame.get_player_status())));
  }
  ScreenManager::current()->set_screen_fade(std::unique_ptr<ScreenFade>(new FadeIn(1)));
  end_seq_started = false;
}

void
GameSession::leave()
{
}

void
GameSession::update(float elapsed_time)
{
  // Set active flag
  if(!active)
  {
    active = true;
  }
  // handle controller
  auto controller = InputManager::current()->get_controller(); 
  if(controller->pressed(Controller::ESCAPE) || 
     controller->pressed(Controller::START))
  {
    on_escape_press();
  }

  if(controller->pressed(Controller::CHEAT_MENU) && g_config->developer_mode)
  {
    if (!MenuManager::instance().is_active())
    {
      game_pause = true;
      MenuManager::instance().set_menu(MenuStorage::CHEAT_MENU);
    }
  }

  process_events();

  // Unpause the game if the menu has been closed
  if (game_pause && !MenuManager::instance().is_active()) {
    ScreenManager::current()->set_speed(speed_before_pause);
    SoundManager::current()->resume_music();
    SoundManager::current()->resume_sounds();
    currentsector->play_looping_sounds();
    game_pause = false;
  }

  check_end_conditions();

  // respawning in new sector?
  if(!newsector.empty() && !newspawnpoint.empty()) {
    auto sector = level->get_sector(newsector);
    if(sector == 0) {
      log_warning << "Sector '" << newsector << "' not found" << std::endl;
      sector = level->get_sector("main");
    }
    currentsector->stop_looping_sounds();
    sector->activate(newspawnpoint);
    sector->play_music(LEVEL_MUSIC);
    currentsector = sector;
    currentsector->play_looping_sounds();
    //Keep persistent across sectors
    if(edit_mode)
      currentsector->get_players()[0]->set_edit_mode(edit_mode);
    newsector = "";
    newspawnpoint = "";
  }

  // Update the world state and all objects in the world
  if(!game_pause) {
    // Update the world
    if (!end_sequence) {
      play_time += elapsed_time; //TODO: make sure we don't count cutscene time
      level->stats.time = play_time;
      currentsector->update(elapsed_time);
    } else {
      if (!end_sequence->is_tux_stopped()) {
        currentsector->update(elapsed_time);
      } else {
        end_sequence->update(elapsed_time);
      }
    }
  }

  if(currentsector == NULL)
    return;

  // update sounds
  if (currentsector->camera) SoundManager::current()->set_listener_position(currentsector->camera->get_center());

  /* Handle music: */
  if (end_sequence)
    return;

  if(currentsector->player->invincible_timer.started()) {
    if(currentsector->player->invincible_timer.get_timeleft() <=
       TUX_INVINCIBLE_TIME_WARNING) {
      currentsector->play_music(HERRING_WARNING_MUSIC);
    } else {
      currentsector->play_music(HERRING_MUSIC);
    }
  } else if(currentsector->get_music_type() != LEVEL_MUSIC) {
    currentsector->play_music(LEVEL_MUSIC);
  }
  if (reset_button) {
    reset_button = false;
    reset_level();
    restart_level();
  }
}

void
GameSession::finish(bool win)
{
  if(end_seq_started)
    return;
  end_seq_started = true;

  using namespace worldmap;

  if (edit_mode) {
    force_ghost_mode();
    return;
  }

  if(win) {
    if(WorldMap::current())
    {
      WorldMap::current()->finished_level(level.get());
    }

    if (LevelsetScreen::current())
    {
      LevelsetScreen::current()->finished_level(win);
    }
  }

  ScreenManager::current()->pop_screen();
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
GameSession::get_working_directory() const
{
  return FileSystem::dirname(levelfile);
}

void
GameSession::start_sequence(Sequence seq)
{
  // do not play sequences when in edit mode
  if (edit_mode) {
    force_ghost_mode();
    return;
  }

  // handle special "stoptux" sequence
  if (seq == SEQ_STOPTUX) {
    if (!end_sequence) {
      log_warning << "Final target reached without an active end sequence" << std::endl;
      this->start_sequence(SEQ_ENDSEQUENCE);
    }
    if (end_sequence) end_sequence->stop_tux();
    return;
  }

  // abort if a sequence is already playing
  if (end_sequence)
    return;

  if (seq == SEQ_ENDSEQUENCE) {
    if (currentsector->get_players()[0]->get_physic().get_velocity_x() < 0) {
      end_sequence = std::make_shared<EndSequenceWalkLeft>();
    } else {
      end_sequence = std::make_shared<EndSequenceWalkRight>();
    }
  } else if (seq == SEQ_FIREWORKS) {
    end_sequence = std::make_shared<EndSequenceFireworks>();
  } else {
    log_warning << "Unknown sequence '" << (int)seq << "'. Ignoring." << std::endl;
    return;
  }

  /* slow down the game for end-sequence */
  ScreenManager::current()->set_speed(0.5f);

  currentsector->add_object(end_sequence);
  end_sequence->start();

  SoundManager::current()->play_music("music/leveldone.ogg", false);
  currentsector->player->set_winning();

  // Stop all clocks.
  for(const auto& obj : currentsector->gameobjects)
  {
    auto lt = std::dynamic_pointer_cast<LevelTime>(obj);
    if(lt)
      lt->stop();
  }
}

/* (Status): */
void
GameSession::drawstatus(DrawingContext& context)
{
  m_savegame.get_player_status()->draw(context);

  // draw level stats while end_sequence is running
  if (end_sequence) {
    level->stats.draw_endseq_panel(context, best_level_statistics, statistics_backdrop);
  }
}

/* EOF */
