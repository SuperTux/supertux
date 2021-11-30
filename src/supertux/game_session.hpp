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

#ifndef HEADER_SUPERTUX_SUPERTUX_GAME_SESSION_HPP
#define HEADER_SUPERTUX_SUPERTUX_GAME_SESSION_HPP

#include <memory>
#include <vector>
#include <squirrel.h>

#include "math/vector.hpp"
#include "squirrel/squirrel_scheduler.hpp"
#include "supertux/game_object.hpp"
#include "supertux/game_session_recorder.hpp"
#include "supertux/player_status.hpp"
#include "supertux/screen.hpp"
#include "supertux/sequence.hpp"
#include "supertux/timer.hpp"
#include "util/currenton.hpp"
#include "video/surface_ptr.hpp"

class CodeController;
class DrawingContext;
class EndSequence;
class Level;
class Player;
class Sector;
class Statistics;
class Savegame;

/** Screen that runs a Level, where Players run and jump through Sectors. */
class GameSession final : public Screen,
                          public GameSessionRecorder,
                          public Currenton<GameSession>
{
public:
  GameSession(const std::string& levelfile, Savegame& savegame, Statistics* statistics = nullptr);

  virtual void draw(Compositor& compositor) override;
  virtual void update(float dt_sec, const Controller& controller) override;
  virtual void setup() override;
  virtual void leave() override;
  virtual IntegrationStatus get_status() const override;

  /** ends the current level */
  void finish(bool win = true);
  void respawn(const std::string& sectorname, const std::string& spawnpointname,
               const bool invincibility = false, const int invincibilityperiod = 0);
  void reset_level();
  void set_start_point(const std::string& sectorname,
                       const std::string& spawnpointname);
  void set_start_pos(const std::string& sectorname, const Vector& pos);
  void set_reset_point(const std::string& sectorname, const Vector& pos);
  std::string get_reset_point_sectorname() const { return m_reset_sector; }

  Vector get_reset_point_pos() const { return m_reset_pos; }
  Sector& get_current_sector() const { return *m_currentsector; }
  Level& get_current_level() const { return *m_level; }

  void start_sequence(Player* caller, Sequence seq, const SequenceData* data = nullptr);

  /**
   * returns the "working directory" usually this is the directory where the
   * currently played level resides. This is used when locating additional
   * resources for the current level/world
   */
  std::string get_working_directory() const;
  int restart_level(bool after_death = false);
  bool reset_button;
  bool reset_checkpoint_button;

  void toggle_pause();
  void abort_level();
  bool is_active() const;

  /** Enters or leaves level editor mode */
  void set_editmode(bool edit_mode = true);

  /** Forces all Players to enter ghost mode */
  void force_ghost_mode();

  Savegame& get_savegame() const { return m_savegame; }

  void set_scheduler(SquirrelScheduler& new_scheduler);

private:
  void check_end_conditions();

  void drawstatus(DrawingContext& context);
  void draw_pause(DrawingContext& context);

  void on_escape_press(bool force_quick_respawn);

private:
  std::unique_ptr<Level> m_level;
  std::unique_ptr<Level> m_old_level;
  SurfacePtr m_statistics_backdrop;

  // scripts
  typedef std::vector<HSQOBJECT> ScriptList;
  ScriptList m_scripts;

  Sector* m_currentsector;

  EndSequence* m_end_sequence;

  bool  m_game_pause;
  float m_speed_before_pause;

  std::string m_levelfile;

  // spawn point (the point where tux respawns at startup). Usually both "main".
  // If m_start_spawnpoint is set, m_start_pos shall not, and vice versa.
  std::string m_start_sector;
  std::string m_start_spawnpoint;
  Vector m_start_pos;

  // reset point (the point where tux respawns if he dies)
  std::string m_reset_sector;
  Vector m_reset_pos;

  // the sector and spawnpoint we should spawn after this frame
  std::string m_newsector;
  std::string m_newspawnpoint;

  // Whether the player had invincibility before spawning in a new sector
  bool m_pastinvincibility;
  int m_newinvincibilityperiod;

  Statistics* m_best_level_statistics;
  Savegame& m_savegame;

  // Note: m_play_time should reset when a level is restarted from the beginning
  //       but NOT if Tux respawns at a checkpoint (for LevelTimes to work)
  float m_play_time; /**< total time in seconds that this session ran interactively */

  bool m_edit_mode; /**< true if GameSession runs in level editor mode */
  bool m_levelintro_shown; /**< true if the LevelIntro screen was already shown */

  int m_coins_at_start; /** How many coins does the player have at the start */
  std::vector<BonusType> m_boni_at_start; /** What boni does the player have at the start */
  std::vector<int> m_max_fire_bullets_at_start; /** How many fire bullets does the player have */
  std::vector<int> m_max_ice_bullets_at_start; /** How many ice bullets does the player have */

  bool m_active; /** Game active? **/

  bool m_end_seq_started;

  std::unique_ptr<GameObject> m_current_cutscene_text;

  Timer m_endsequence_timer;

private:
  GameSession(const GameSession&) = delete;
  GameSession& operator=(const GameSession&) = delete;
};

#endif

/* EOF */
