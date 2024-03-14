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
#include "object/endsequence_walk.hpp"
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

GameSession::GameSession(const std::string& levelfile_, Savegame& savegame, Statistics* statistics,
                         bool preserve_music) :
  reset_button(false),
  reset_checkpoint_button(false),
  m_prevent_death(false),
  m_level(),
  m_statistics_backdrop(Surface::from_file("images/engine/menu/score-backdrop.png")),
  m_scripts(),
  m_currentsector(nullptr),
  m_end_sequence(nullptr),
  m_game_pause(false),
  m_speed_before_pause(ScreenManager::current()->get_speed()),
  m_levelfile(levelfile_),
  m_spawnpoints(),
  m_activated_checkpoint(),
  m_newsector(),
  m_newspawnpoint(),
  m_best_level_statistics(statistics),
  m_savegame(savegame),
  m_play_time(0),
  m_levelintro_shown(false),
  m_coins_at_start(),
  m_boni_at_start(),
  m_max_fire_bullets_at_start(),
  m_max_ice_bullets_at_start(),
  m_active(false),
  m_end_seq_started(false),
  m_pause_target_timer(false),
  m_current_cutscene_text(),
  m_endsequence_timer()
{
  set_start_point("main", "main");

  m_boni_at_start.resize(InputManager::current()->get_num_users(), NO_BONUS);
  m_max_fire_bullets_at_start.resize(InputManager::current()->get_num_users(), 0);
  m_max_ice_bullets_at_start.resize(InputManager::current()->get_num_users(), 0);

  if (restart_level(false, preserve_music) != 0)
    throw std::runtime_error ("Initializing the level failed.");
}

void
GameSession::reset_level()
{
  for (const auto& p : m_currentsector->get_players())
  {
    try
    {
      p->set_bonus(m_boni_at_start.at(p->get_id()));
    }
    catch(const std::out_of_range&)
    {
    }
  }

  PlayerStatus& currentStatus = m_savegame.get_player_status();
  currentStatus.coins = m_coins_at_start;
  currentStatus.bonus = m_boni_at_start;
  currentStatus.max_fire_bullets = m_max_fire_bullets_at_start;
  currentStatus.max_ice_bullets = m_max_ice_bullets_at_start;

  clear_respawn_points();
  m_activated_checkpoint = nullptr;
  m_pause_target_timer = false;
}

int
GameSession::restart_level(bool after_death, bool preserve_music)
{
  const PlayerStatus& currentStatus = m_savegame.get_player_status();
  m_coins_at_start = currentStatus.coins;
  m_max_fire_bullets_at_start = currentStatus.max_fire_bullets;
  m_max_ice_bullets_at_start = currentStatus.max_ice_bullets;
  m_boni_at_start = currentStatus.bonus;

  // Needed for the title screen apparently.
  if (m_currentsector)
  {
    try
    {
      for (const auto& p : m_currentsector->get_players())
      {
        p->set_bonus(m_boni_at_start.at(p->get_id()), false, false);
        m_boni_at_start[p->get_id()] = currentStatus.bonus[p->get_id()];
      }
    }
    catch (const std::out_of_range&)
    {
    }
  }

  m_game_pause   = false;
  m_end_sequence = nullptr;
  m_endsequence_timer.stop();

  InputManager::current()->reset();

  m_currentsector = nullptr;

  const std::string base_dir = FileSystem::dirname(m_levelfile);
  if (base_dir == "./") {
    m_levelfile = FileSystem::basename(m_levelfile);
  }

  try {
    m_level = LevelParser::from_file(m_levelfile, false, false);

    /* Determine the spawnpoint to spawn/respawn Tux to. */
    const GameSession::SpawnPoint* spawnpoint = nullptr;
    if (m_activated_checkpoint && reset_checkpoint_button) // Checkpoint is activated and respawn from it is requested.
    {
      reset_checkpoint_button = false;

      // Attempt to find the last checkpoint's spawnpoint.
      for (int i = static_cast<int>(m_spawnpoints.size()) - 1; i >= 0; i--)
      {
        if (m_spawnpoints.at(i).is_checkpoint) // Checkpoint found.
        {
          spawnpoint = &m_spawnpoints.at(i);

          // Remove any spawnpoints after the checkpoint one.
          if (i < static_cast<int>(m_spawnpoints.size()) - 1)
            m_spawnpoints.erase(m_spawnpoints.begin() + i + 1, m_spawnpoints.end());

          break;
        }
      }

      if (!spawnpoint) // Spawnpoint has not been found.
      {
        throw std::runtime_error("Cannot find the position of the last activated checkpoint.");
      }
    }
    else if (after_death && m_spawnpoints.size() > 1) // Respawn from the last respawn position, because Tux is respawning.
    {
      spawnpoint = &get_last_spawnpoint();
    }
    else // Respawn from the start position.
    {
      spawnpoint = &m_spawnpoints.front();

      m_play_time = 0; // Reset play time.
    }

    /* Perform the respawn from the chosen spawnpoint. */
    // Load the spawn sector.
    m_currentsector = m_level->get_sector(spawnpoint->sector);
    if (!m_currentsector)
    {
      throw std::runtime_error("Couldn't find sector '" + spawnpoint->sector + "' to spawn/respawn Tux.");
    }
    // Activate on either the spawnpoint (if set), or the spawn position.
    if (spawnpoint->spawnpoint.empty())
    {
      m_currentsector->activate(spawnpoint->position);
    }
    else
    {
      m_currentsector->activate(spawnpoint->spawnpoint);
    }
  }
  catch (std::exception& e) {
    log_fatal << "Couldn't start level: " << e.what() << std::endl;
    ScreenManager::current()->pop_screen();
    return (-1);
  }

  if (!preserve_music)
  {
    auto& music_object = m_currentsector->get_singleton_by_type<MusicObject>();
    if (after_death == true) {
      music_object.resume_music();
    } else {
      SoundManager::current()->stop_music();
      music_object.play_music(LEVEL_MUSIC);
    }
  }

  auto level_times = m_currentsector->get_objects_by_type<LevelTime>();
  auto it = level_times.begin();

  while (it != level_times.end())
  {
    it->set_time(it->get_time() - m_play_time);
    it++;
  }

  return (0);
}

void
GameSession::on_escape_press(bool force_quick_respawn)
{
  auto players = m_currentsector->get_players();

  int alive = m_currentsector->get_object_count<Player>([](const Player& p) {
    return !p.is_dead() && !p.is_dying();
  });

  if ((!alive && (m_play_time > 2.0f || force_quick_respawn)) || m_end_sequence)
  {
    // Let the timers run out, we fast-forward them to force past a sequence.
    if (m_end_sequence)
    {
      if (m_end_sequence->is_running())
      {
        m_end_sequence->stop();
      }
      else
      {
        m_endsequence_timer.start(FLT_EPSILON);
      }
    }

    for (auto* player : players)
      player->m_dying_timer.start(FLT_EPSILON);

    return;   // Don't let the player open the menu, when Tux is dying.
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
  // Pause.
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

  // Unpause is done in update() after the menu is processed.
}

void
GameSession::abort_level()
{
  MenuManager::instance().clear_menu_stack();
  ScreenManager::current()->pop_screen();

  for (const auto& p : m_currentsector->get_players())
  {
    try
    {
      p->set_bonus(m_boni_at_start.at(p->get_id()));
    }
    catch(const std::out_of_range&)
    {
    }
  }

  PlayerStatus& currentStatus = m_savegame.get_player_status();
  currentStatus.coins = m_coins_at_start;
  currentStatus.max_fire_bullets = m_max_fire_bullets_at_start;
  currentStatus.max_ice_bullets = m_max_ice_bullets_at_start;
  SoundManager::current()->stop_sounds();
}

bool
GameSession::is_active() const
{
  return !m_game_pause && m_active && !(m_end_sequence && m_end_sequence->is_running());
}

void
GameSession::check_end_conditions()
{
  bool all_dead = true;
  for (const auto* p : m_currentsector->get_players())
    if (!(all_dead &= p->is_dead()))
      break;

  bool all_dead_or_winning = true;
  for (const auto* p : m_currentsector->get_players())
    if (!(all_dead_or_winning &= (p->is_dead() || p->is_dying() || p->is_winning())))
      break;

  /* End of level? */
  if (m_endsequence_timer.check() || (all_dead_or_winning && m_end_sequence && m_endsequence_timer.get_period() > 0.f)) {
    m_endsequence_timer.stop();
    for (auto* p : m_currentsector->get_players())
      p->set_winning();
    start_sequence(nullptr, Sequence::SEQ_ENDSEQUENCE);
  } else if (m_end_sequence && m_end_sequence->is_done()) {
    finish(true);
  } else if (!m_end_sequence && all_dead) {
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
    Rectf(context.get_rect()),
    Color(0.0f, 0.0f, 0.0f, 0.25f),
    LAYER_FOREGROUND1);
}

void
GameSession::setup()
{
  if (m_currentsector == nullptr)
    return;

  if (m_currentsector != Sector::current()) {
    m_currentsector->activate(m_currentsector->get_players()[0]->get_pos());
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
  // Set active flag.
  if (!m_active)
  {
    m_active = true;
  }
  // Handle controller.

  if (controller.pressed_any(Control::ESCAPE, Control::START))
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
  // design choice, if you prefer it not to animate when paused, add `if (!m_game_pause)`).
  m_level->m_stats.update_timers(dt_sec);

  // Unpause the game if the menu has been closed.
  if (m_game_pause && !MenuManager::instance().is_active()) {
    ScreenManager::current()->set_speed(m_speed_before_pause);
    SoundManager::current()->resume_music();
    SoundManager::current()->resume_sounds();
    assert(m_currentsector != nullptr);
    m_currentsector->play_looping_sounds();
    m_game_pause = false;
  }

  check_end_conditions();

  // Respawning in new sector?
  if (!m_newsector.empty() && !m_newspawnpoint.empty()) {
    auto sector = m_level->get_sector(m_newsector);
    std::string current_music = m_currentsector->get_singleton_by_type<MusicObject>().get_music();
    if (sector == nullptr) {
      log_warning << "Sector '" << m_newsector << "' not found" << std::endl;
      sector = m_level->get_sector(m_spawnpoints.at(0).sector); // Assign start sector.
    }
    assert(m_currentsector != nullptr);
    m_currentsector->stop_looping_sounds();

    sector->activate(m_newspawnpoint);

    // Start the new sector's music only if it's different from the current one.
    if (current_music != sector->get_singleton_by_type<MusicObject>().get_music())
      sector->get_singleton_by_type<MusicObject>().play_music(LEVEL_MUSIC);

    m_currentsector = sector;
    m_currentsector->play_looping_sounds();

    m_newsector = "";
    m_newspawnpoint = "";
  }

  // Update the world state and all objects in the world.
  if (!m_game_pause) {
    assert(m_currentsector != nullptr);
    // Update the world.
    if (!m_end_sequence || !m_end_sequence->is_running()) {
      if (!m_level->m_is_in_cutscene && !m_pause_target_timer)
      {
        m_play_time += dt_sec;
        m_level->m_stats.finish(m_play_time);
      }
      m_currentsector->update(dt_sec);
    } else {
      bool are_all_stopped = true;

      for (const auto& player : m_currentsector->get_players())
      {
        if (!(m_end_sequence->is_tux_stopped(player->get_id())
            || player->get_ending_direction() == 0))
        {
          are_all_stopped = false;
          break;
        }
      }

      if (!are_all_stopped) {
        m_currentsector->update(dt_sec);
      } else {
        m_end_sequence->update(dt_sec);
      }
    }
  }

  if (m_currentsector == nullptr)
    return;

  // Update sounds.
  SoundManager::current()->set_listener_position(m_currentsector->get_camera().get_center());

  /* Handle music: */
  if (m_end_sequence)
    return;

  bool invincible_timer_started = false;
  float max_invincible_timer_left = 0.f;

  for (const auto* p : m_currentsector->get_players())
  {
    invincible_timer_started |= (p->m_invincible_timer.started() && !p->is_winning());
    max_invincible_timer_left = std::max(max_invincible_timer_left, p->m_invincible_timer.get_timeleft());
  }

  auto& music_object = m_currentsector->get_singleton_by_type<MusicObject>();
  if (invincible_timer_started) {
    if (max_invincible_timer_left <= TUX_INVINCIBLE_TIME_WARNING) {
      if (music_object.get_music_type() != HERRING_WARNING_MUSIC)
        music_object.play_music(HERRING_WARNING_MUSIC);
    } else {
      if (music_object.get_music_type() != HERRING_MUSIC)
        music_object.play_music(HERRING_MUSIC);
    }
  } else if (music_object.get_music_type() != LEVEL_MUSIC) {
    music_object.play_music(LEVEL_MUSIC);
  }
  if (reset_button) {
    reset_button = false;
    reset_level();
    restart_level();
  } else if(reset_checkpoint_button) {
    for (auto* p : m_currentsector->get_players())
      p->kill(true);
  }
}

IntegrationStatus
GameSession::get_status() const
{
  IntegrationStatus status;
  status.m_details.push_back(Editor::current() ? "Testing" : "Playing");
  if (!Editor::current() || !g_config->hide_editor_levelnames)
  {
    const std::string label = get_current_level().is_worldmap() ? "In worldmap: " : "In level: ";
    status.m_details.push_back(label + get_current_level().get_name());
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

  if (win) {
    if (WorldMapSector::current())
    {
      WorldMapSector::current()->finished_level(m_level.get());
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
  m_newsector = sector;
  m_newspawnpoint = spawnpoint;
}

void
GameSession::set_start_point(const std::string& sector,
                             const std::string& spawnpoint)
{
  if (!m_spawnpoints.empty()) m_spawnpoints.erase(m_spawnpoints.begin());
  m_spawnpoints.insert(m_spawnpoints.begin(), { sector, spawnpoint });
}

void
GameSession::set_start_pos(const std::string& sector, const Vector& pos)
{
  if (!m_spawnpoints.empty()) m_spawnpoints.erase(m_spawnpoints.begin());
  m_spawnpoints.insert(m_spawnpoints.begin(), { sector, pos });
}

void
GameSession::set_respawn_point(const std::string& sector,
                               const std::string& spawnpoint)
{
  m_spawnpoints.push_back({ sector, spawnpoint });
}

void
GameSession::set_respawn_pos(const std::string& sector, const Vector& pos)
{
  m_spawnpoints.push_back({ sector, pos });
}

void
GameSession::clear_respawn_points()
{
  // Delete all respawn points (all, other than the start one).
  if (m_spawnpoints.size() > 1)
    m_spawnpoints.erase(m_spawnpoints.begin() + 1, m_spawnpoints.end());
}

const GameSession::SpawnPoint&
GameSession::get_last_spawnpoint() const
{
  return m_spawnpoints.back();
}

void
GameSession::set_checkpoint_pos(const std::string& sector, const Vector& pos)
{
  m_spawnpoints.push_back({ sector, pos, true });
  m_activated_checkpoint = &m_spawnpoints.back();
}

const GameSession::SpawnPoint*
GameSession::get_active_checkpoint_spawnpoint() const
{
  return m_activated_checkpoint;
}

std::string
GameSession::get_working_directory() const
{
  return FileSystem::dirname(m_levelfile);
}

bool
GameSession::has_active_sequence() const
{
  return m_end_sequence;
}

void
GameSession::start_sequence(Player* caller, Sequence seq, const SequenceData* data)
{
  // Handle special "stoptux" sequence.
  if (seq == SEQ_STOPTUX) {
    if (!m_end_sequence) {
      log_warning << "Final target reached without an active end sequence" << std::endl;
      start_sequence(caller, SEQ_ENDSEQUENCE);
    }

    if (m_end_sequence)
    {
      if (caller)
      {
        m_end_sequence->stop_tux(caller->get_id());
      }
      else
      {
        for (const auto* player : Sector::get().get_players())
          m_end_sequence->stop_tux(player->get_id());
      }
    }
    return;
  }

  if (caller)
    caller->set_winning();

  int remaining_players = get_current_sector().get_object_count<Player>([](const Player& p){
    return !p.is_dead() && !p.is_dying() && !p.is_winning();
  });

  // Abort if a sequence is already playing.
  if (m_end_sequence && m_end_sequence->is_running())
    return;

  // Set the sequence to prepare it.
  if (!m_end_sequence) {
    std::unique_ptr<EndSequence> end_sequence;
    if (seq == SEQ_ENDSEQUENCE) {
      end_sequence = std::make_unique<EndSequenceWalk>();
    } else if (seq == SEQ_FIREWORKS) {
      end_sequence = std::make_unique<EndSequenceFireworks>();
    } else {
      log_warning << "Unknown sequence '" << static_cast<int>(seq) << "'. Ignoring." << std::endl;
      return;
    }

    m_end_sequence = static_cast<EndSequence*>(&m_currentsector->add_object(std::move(end_sequence)));
  }

  if (caller)
  {
    caller->set_ending_direction((caller->get_physic().get_velocity_x() < 0) ? -1 : 1);
    caller->set_controller(m_end_sequence->get_controller(caller->get_id()));
    caller->set_speedlimit(230); // MAX_WALK_XM
  }

  // Don't play the prepared sequence if there are more players that are still playing.
  if (remaining_players > 0)
  {
    if (!m_endsequence_timer.started())
      m_endsequence_timer.start(10.f);

    return;
  }

  m_endsequence_timer.stop();

  if (const auto& worldmap_sector = worldmap::WorldMapSector::current())
  {
    if (data != nullptr)
    {
      if (!data->fade_tilemap.empty())
      {
        worldmap_sector->set_initial_fade_tilemap(data->fade_tilemap, data->fade_type);
      }
      if (!data->spawnpoint.empty())
      {
        worldmap_sector->get_worldmap().set_initial_spawnpoint(data->spawnpoint);
      }
    }
  }

  /* Slow down the game for end-sequence. */
  ScreenManager::current()->set_speed(0.5f);

  m_end_sequence->start();

  SoundManager::current()->play_music("music/misc/leveldone.ogg", false);
  for (auto* p : m_currentsector->get_players())
  {
    p->set_winning();
    p->set_controller(m_end_sequence->get_controller(p->get_id()));
    p->set_speedlimit(230); // MAX_WALK_XM.
  }

  // Stop all clocks.
  for (LevelTime& lt : m_currentsector->get_objects_by_type<LevelTime>())
  {
    lt.stop();
  }
}
void 
GameSession::set_target_timer_paused(bool paused)
{
  m_pause_target_timer = paused;
  for (LevelTime& lt : m_currentsector->get_objects_by_type<LevelTime>())
  {
    if(paused)
      lt.stop();
    else
      lt.start();
  }
}

/* (Status): */
void
GameSession::drawstatus(DrawingContext& context)
{
  // Draw level stats while end_sequence is running.
  if (m_end_sequence && m_end_sequence->is_running()) {
    m_level->m_stats.draw_endseq_panel(context, m_best_level_statistics, m_statistics_backdrop, m_level->m_target_time);
  }

  m_level->m_stats.draw_ingame_stats(context, m_game_pause);
}

/* EOF */
