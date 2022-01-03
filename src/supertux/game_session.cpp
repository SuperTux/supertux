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

#include <cfloat>

#include "audio/sound_manager.hpp"
#include "control/input_manager.hpp"
#include "editor/editor.hpp"
#include "gui/menu_manager.hpp"
#include "math/vector.hpp"
#include "object/camera.hpp"
#include "object/endsequence_fireworks.hpp"
#include "object/endsequence_walkleft.hpp"
#include "object/endsequence_walkright.hpp"
#include "object/level_time.hpp"
#include "object/music_object.hpp"
#include "object/player.hpp"
#include "sdk/integration.hpp"
#include "supertux/fadetoblack.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/level.hpp"
#include "supertux/level_parser.hpp"
#include "supertux/levelintro.hpp"
#include "supertux/levelset_screen.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/savegame.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/sector.hpp"
#include "util/file_system.hpp"
#include "video/compositor.hpp"
#include "video/drawing_context.hpp"
#include "video/surface.hpp"
#include "worldmap/worldmap.hpp"

GameSession::GameSession(const std::string& levelfile_, Savegame& savegame, Statistics* statistics) :
  GameSessionRecorder(),
  reset_button(false),
  reset_checkpoint_button(false),
  m_level(),
  m_old_level(),
  m_statistics_backdrop(Surface::from_file("images/engine/menu/score-backdrop.png")),
  m_scripts(),
  m_currentsector(nullptr),
  m_end_sequence(nullptr),
  m_game_pause(false),
  m_speed_before_pause(ScreenManager::current()->get_speed()),
  m_levelfile(levelfile_),
  m_start_sector("main"),
  m_start_spawnpoint("main"),
  m_start_pos(0.0f, 0.0f),
  m_reset_sector(),
  m_reset_pos(0.0f, 0.0f),
  m_newsector(),
  m_newspawnpoint(),
  m_pastinvincibility(false),
  m_newinvincibilityperiod(0),
  m_best_level_statistics(statistics),
  m_savegame(savegame),
  m_play_time(0),
  m_edit_mode(false),
  m_levelintro_shown(false),
  m_coins_at_start(),
  m_bonus_at_start(),
  m_max_fire_bullets_at_start(),
  m_max_ice_bullets_at_start(),
  m_active(false),
  m_end_seq_started(false),
  m_current_cutscene_text()
{
  if (restart_level() != 0)
    throw std::runtime_error ("Initializing the level failed.");
}

void
GameSession::reset_level()
{
  m_currentsector->get_player().set_bonus(m_bonus_at_start);
  PlayerStatus& currentStatus = m_savegame.get_player_status();
  currentStatus.coins = m_coins_at_start;
  currentStatus.max_fire_bullets = m_max_fire_bullets_at_start;
  currentStatus.max_ice_bullets = m_max_ice_bullets_at_start;
  m_reset_sector = "";
  m_reset_pos = Vector(0.0f, 0.0f);
}

int
GameSession::restart_level(bool after_death)
{
  const PlayerStatus& currentStatus = m_savegame.get_player_status();
  m_coins_at_start = currentStatus.coins;
  m_bonus_at_start = currentStatus.bonus;
  m_max_fire_bullets_at_start = currentStatus.max_fire_bullets;
  m_max_ice_bullets_at_start = currentStatus.max_ice_bullets;

  if (m_edit_mode) {
    force_ghost_mode();
    return (-1);
  }

  m_game_pause   = false;
  m_end_sequence = nullptr;

  InputManager::current()->reset();

  m_currentsector = nullptr;

  const std::string base_dir = FileSystem::dirname(m_levelfile);
  if (base_dir == "./") {
    m_levelfile = FileSystem::basename(m_levelfile);
  }

  try {
    m_old_level = std::move(m_level);
    m_level = LevelParser::from_file(m_levelfile, false, false);

    if (!m_reset_sector.empty()) {
      m_currentsector = m_level->get_sector(m_reset_sector);
      if (!m_currentsector) {
        std::stringstream msg;
        msg << "Couldn't find sector '" << m_reset_sector << "' for resetting tux.";
        throw std::runtime_error(msg.str());
      }
      m_currentsector->activate(m_reset_pos);
    } else {
      m_currentsector = m_level->get_sector(m_start_sector);
      if (!m_currentsector)
        throw std::runtime_error("Couldn't find main sector");
      m_play_time = 0;
      if (m_start_spawnpoint.empty()) {
        m_currentsector->activate(m_start_pos);
      } else {
        m_currentsector->activate(m_start_spawnpoint);
      }
    }
  } catch(std::exception& e) {
    log_fatal << "Couldn't start level: " << e.what() << std::endl;
    ScreenManager::current()->pop_screen();
    return (-1);
  }

  auto& music_object = m_currentsector->get_singleton_by_type<MusicObject>();
  if (after_death == true) {
    music_object.resume_music();
  } else {
    SoundManager::current()->stop_music();
    music_object.play_music(LEVEL_MUSIC);
  }

  start_recording();

  return (0);
}

void
GameSession::on_escape_press(bool force_quick_respawn)
{
  if ((m_currentsector->get_player().is_dying() && (m_play_time > 2.0f
      || force_quick_respawn))
    || m_end_sequence)
  {
    // Let the timers run out, we fast-forward them to force past a sequence
    if (m_end_sequence)
      m_end_sequence->stop();

    m_currentsector->get_player().m_dying_timer.start(FLT_EPSILON);
    return;   // don't let the player open the menu, when Tux is dying
  }

  if (m_level->m_is_in_cutscene && !m_level->m_skip_cutscene)
  {
    m_level->m_skip_cutscene = true;
    return;
  }

  if (m_level->m_suppress_pause_menu && ScreenManager::current()->has_pending_fadeout())
    return;

  if (!m_level->m_suppress_pause_menu) {
    toggle_pause();
  } else {
	  abort_level();
  }
}

void
GameSession::toggle_pause()
{
  // pause
  if (!m_game_pause && !MenuManager::instance().is_active())
  {
    m_speed_before_pause = ScreenManager::current()->get_speed();
    ScreenManager::current()->set_speed(0);
    MenuManager::instance().set_menu(MenuStorage::GAME_MENU);
    SoundManager::current()->pause_sounds();
    m_currentsector->stop_looping_sounds();
    SoundManager::current()->pause_music();
    m_game_pause = true;
  }

  // unpause is done in update() after the menu is processed
}

void
GameSession::abort_level()
{
  MenuManager::instance().clear_menu_stack();
  ScreenManager::current()->pop_screen();
  m_currentsector->get_player().set_bonus(m_bonus_at_start);
  PlayerStatus& currentStatus = m_savegame.get_player_status();
  currentStatus.coins = m_coins_at_start;
  currentStatus.max_fire_bullets = m_max_fire_bullets_at_start;
  currentStatus.max_ice_bullets = m_max_ice_bullets_at_start;
  SoundManager::current()->stop_sounds();
}

bool
GameSession::is_active() const
{
  return !m_game_pause && m_active && !m_end_sequence;
}

void
GameSession::set_editmode(bool edit_mode_)
{
  if (m_edit_mode == edit_mode_) return;
  m_edit_mode = edit_mode_;

  m_currentsector->get_player().set_edit_mode(edit_mode_);

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
  m_currentsector->get_player().set_ghost_mode(true);
}

void
GameSession::check_end_conditions()
{
  Player& tux = m_currentsector->get_player();

  /* End of level? */
  if (m_end_sequence && m_end_sequence->is_done()) {
    finish(true);
  } else if (!m_end_sequence && tux.is_dead()) {
    restart_level(true);
  }
}

void
GameSession::draw(Compositor& compositor)
{
  auto& context = compositor.make_context();

  m_currentsector->draw(context);
  drawstatus(context);

  if (m_game_pause)
    draw_pause(context);
}

void
GameSession::draw_pause(DrawingContext& context)
{
  context.color().draw_filled_rect(
    Rectf(0, 0, static_cast<float>(context.get_width()), static_cast<float>(context.get_height())),
    Color(0.0f, 0.0f, 0.0f, 0.25f),
    LAYER_FOREGROUND1);
}

void
GameSession::setup()
{
  if (m_currentsector == nullptr)
    return;

  if (m_currentsector != Sector::current()) {
    m_currentsector->activate(m_currentsector->get_player().get_pos());
  }
  m_currentsector->get_singleton_by_type<MusicObject>().play_music(LEVEL_MUSIC);

  int total_stats_to_be_collected = m_level->m_stats.m_total_coins + m_level->m_stats.m_total_badguys + m_level->m_stats.m_total_secrets;
  if ((!m_levelintro_shown) && (total_stats_to_be_collected > 0)) {
    m_levelintro_shown = true;
    m_active = false;
    ScreenManager::current()->push_screen(std::make_unique<LevelIntro>(*m_level, m_best_level_statistics, m_savegame.get_player_status()));
  }
  ScreenManager::current()->set_screen_fade(std::make_unique<FadeToBlack>(FadeToBlack::FADEIN, 1.0f));
  m_end_seq_started = false;
}

void
GameSession::leave()
{
}

void
GameSession::update(float dt_sec, const Controller& controller)
{
  // Set active flag
  if (!m_active)
  {
    m_active = true;
  }
  // handle controller

  if (controller.pressed(Control::ESCAPE) ||
      controller.pressed(Control::START))
  {
    on_escape_press(controller.hold(Control::LEFT)
      || controller.hold(Control::RIGHT));
  }

  if (controller.pressed(Control::CHEAT_MENU) &&
      (g_config->developer_mode || (Editor::current() && Editor::current()->is_testing_level()))
     )
  {
    if (!MenuManager::instance().is_active())
    {
      toggle_pause();
      MenuManager::instance().set_menu(MenuStorage::CHEAT_MENU);
    }
  }

  if (controller.pressed(Control::DEBUG_MENU) && g_config->developer_mode)
  {
    if (!MenuManager::instance().is_active())
    {
      toggle_pause();
      MenuManager::instance().set_menu(MenuStorage::DEBUG_MENU);
    }
  }

  // Animate the full-completion stats stuff - do this even when the game isn't paused (that's a
  // design choice, if you prefer it not to animate when paused, add `if (!m_game_pause)`)
  m_level->m_stats.update_timers(dt_sec);

  process_events();

  // Unpause the game if the menu has been closed
  if (m_game_pause && !MenuManager::instance().is_active()) {
    ScreenManager::current()->set_speed(m_speed_before_pause);
    SoundManager::current()->resume_music();
    SoundManager::current()->resume_sounds();
    assert(m_currentsector != nullptr);
    m_currentsector->play_looping_sounds();
    m_game_pause = false;
  }

  check_end_conditions();

  // respawning in new sector?
  if (!m_newsector.empty() && !m_newspawnpoint.empty()) {
    auto sector = m_level->get_sector(m_newsector);
    if (sector == nullptr) {
      log_warning << "Sector '" << m_newsector << "' not found" << std::endl;
      sector = m_level->get_sector(m_start_sector);
    }
    assert(m_currentsector != nullptr);
    m_currentsector->stop_looping_sounds();
    sector->activate(m_newspawnpoint);
    sector->get_singleton_by_type<MusicObject>().play_music(LEVEL_MUSIC);
    m_currentsector = sector;
    m_currentsector->play_looping_sounds();

    if (is_playing_demo())
    {
      reset_demo_controller();
    }
    //Keep persistent across sectors
    if (m_edit_mode)
      m_currentsector->get_player().set_edit_mode(m_edit_mode);
    m_newsector = "";
    m_newspawnpoint = "";
    // retain invincibility if the player has it
    if (m_pastinvincibility) {
      m_currentsector->get_player().m_invincible_timer.start(static_cast<float>(m_newinvincibilityperiod));
    }
  }

  // Update the world state and all objects in the world
  if (!m_game_pause) {
    assert(m_currentsector != nullptr);
    // Update the world
    if (!m_end_sequence) {
      if (!m_level->m_is_in_cutscene)
      {
        m_play_time += dt_sec;
        m_level->m_stats.finish(m_play_time);
      }
      m_currentsector->update(dt_sec);
    } else {
      if (!m_end_sequence->is_tux_stopped()) {
        m_currentsector->update(dt_sec);
      } else {
        m_end_sequence->update(dt_sec);
      }
    }
  }

  if (m_currentsector == nullptr)
    return;

  // update sounds
  SoundManager::current()->set_listener_position(m_currentsector->get_camera().get_center());

  /* Handle music: */
  if (m_end_sequence)
    return;

  if (m_currentsector->get_player().m_invincible_timer.started()) {
    if (m_currentsector->get_player().m_invincible_timer.get_timeleft() <=
       TUX_INVINCIBLE_TIME_WARNING) {
      m_currentsector->get_singleton_by_type<MusicObject>().play_music(HERRING_WARNING_MUSIC);
    } else {
      m_currentsector->get_singleton_by_type<MusicObject>().play_music(HERRING_MUSIC);
    }
  } else if (m_currentsector->get_singleton_by_type<MusicObject>().get_music_type() != LEVEL_MUSIC) {
    m_currentsector->get_singleton_by_type<MusicObject>().play_music(LEVEL_MUSIC);
  }
  if (reset_button) {
    reset_button = false;
    reset_level();
    restart_level();
  } else if(reset_checkpoint_button) {
    reset_checkpoint_button = false;

    get_current_sector().get_player().kill(true);
  }
}

IntegrationStatus
GameSession::get_status() const
{
  IntegrationStatus status;
  status.m_details.push_back(Editor::current() ? "Testing" : "Playing");
  if (!Editor::current() || g_config->hide_editor_levelnames)
  {
    if (get_current_level().is_worldmap())
    {
      status.m_details.push_back("In worldmap: " + get_current_level().get_name());
    }
    else
    {
      status.m_details.push_back("In level: " + get_current_level().get_name());
    }
  }
  return status;
}

void
GameSession::finish(bool win)
{
  if (m_end_seq_started)
    return;
  m_end_seq_started = true;

  using namespace worldmap;

  if (m_edit_mode) {
    force_ghost_mode();
    return;
  }

  if (win) {
    if (WorldMap::current())
    {
      WorldMap::current()->finished_level(m_level.get());
    }

    if (LevelsetScreen::current())
    {
      LevelsetScreen::current()->finished_level(win);
    }
  }

  ScreenManager::current()->pop_screen();
}

void
GameSession::respawn(const std::string& sector, const std::string& spawnpoint,
                     const bool invincibility, const int invincibilityperiod)
{
  m_newsector = sector;
  m_newspawnpoint = spawnpoint;
  m_pastinvincibility = invincibility;
  m_newinvincibilityperiod = invincibilityperiod;
}

void
GameSession::set_start_point(const std::string& sector,
                             const std::string& spawnpoint)
{
  m_start_sector = sector;
  m_start_spawnpoint = spawnpoint;
  m_start_pos = Vector(0.0f, 0.0f);
}

void
GameSession::set_start_pos(const std::string& sector,
                           const Vector& pos)
{
  m_start_sector = sector;
  m_start_spawnpoint = "";
  m_start_pos = pos;
}

void
GameSession::set_reset_point(const std::string& sector, const Vector& pos)
{
  m_reset_sector = sector;
  m_reset_pos = pos;
}

std::string
GameSession::get_working_directory() const
{
  return FileSystem::dirname(m_levelfile);
}

void
GameSession::start_sequence(Sequence seq, const SequenceData* data)
{
  // do not play sequences when in edit mode
  if (m_edit_mode) {
    force_ghost_mode();
    return;
  }

  // handle special "stoptux" sequence
  if (seq == SEQ_STOPTUX) {
    if (!m_end_sequence) {
      log_warning << "Final target reached without an active end sequence" << std::endl;
      start_sequence(SEQ_ENDSEQUENCE);
    }
    if (m_end_sequence) m_end_sequence->stop_tux();
    return;
  }

  // abort if a sequence is already playing
  if (m_end_sequence)
    return;

  std::unique_ptr<EndSequence> end_sequence;
  if (seq == SEQ_ENDSEQUENCE) {
    if (m_currentsector->get_player().get_physic().get_velocity_x() < 0) {
      end_sequence = std::make_unique<EndSequenceWalkLeft>();
    } else {
      end_sequence = std::make_unique<EndSequenceWalkRight>();
    }
  } else if (seq == SEQ_FIREWORKS) {
    end_sequence = std::make_unique<EndSequenceFireworks>();
  } else {
    log_warning << "Unknown sequence '" << static_cast<int>(seq) << "'. Ignoring." << std::endl;
    return;
  }

  if (const auto& worldmap = worldmap::WorldMap::current())
  {
    if (data != nullptr)
    {
      if (!data->fade_tilemap.empty())
      {
        worldmap->set_initial_fade_tilemap(data->fade_tilemap, data->fade_type);
      }
      if (!data->spawnpoint.empty())
      {
        worldmap->set_initial_spawnpoint(data->spawnpoint);
      }
    }
  }

  /* slow down the game for end-sequence */
  ScreenManager::current()->set_speed(0.5f);

  m_end_sequence = static_cast<EndSequence*>(&m_currentsector->add_object(std::move(end_sequence)));
  m_end_sequence->start();

  SoundManager::current()->play_music("music/misc/leveldone.ogg", false);
  m_currentsector->get_player().set_winning();

  // Stop all clocks.
  for (const auto& obj : m_currentsector->get_objects())
  {
    auto lt = dynamic_cast<LevelTime*>(obj.get());
    if (lt)
      lt->stop();
  }
}

/* (Status): */
void
GameSession::drawstatus(DrawingContext& context)
{
  // draw level stats while end_sequence is running
  if (m_end_sequence) {
    m_level->m_stats.draw_endseq_panel(context, m_best_level_statistics, m_statistics_backdrop, m_level->m_target_time);
  }

  m_level->m_stats.draw_ingame_stats(context, m_game_pause);
}

/* EOF */
