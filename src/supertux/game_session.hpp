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

#include "supertux/screen.hpp"
#include "util/currenton.hpp"

#include <memory>
#include <unordered_map>
#include <vector>

#include <simplesquirrel/table.hpp>

#include "math/vector.hpp"
#include "squirrel/squirrel_scheduler.hpp"
#include "squirrel/squirrel_util.hpp"
#include "supertux/game_object.hpp"
#include "supertux/level.hpp"
#include "supertux/screen_fade.hpp"
#include "supertux/sequence.hpp"
#include "supertux/timer.hpp"
#include "video/surface_ptr.hpp"

class CodeController;
class DrawingContext;
class EndSequence;
class GameServerUser;
class LevelIntro;
class Player;
class Sector;
class Statistics;
class Savegame;

namespace network {
class Host;
} // namespace network

/** Screen that runs a Level, where Players run and jump through Sectors. */
class GameSession final : public Screen,
                          public Currenton<GameSession>
{
public:
  struct SpawnPoint
  {
  public:
    /* If a spawnpoint is set, the spawn position shall not, and vice versa. */
    SpawnPoint(const std::string& sector,
               const Vector& position,
               bool is_checkpoint = false);
    SpawnPoint(const std::string& sector,
               const std::string& spawnpoint,
               bool is_checkpoint = false);
    SpawnPoint(const std::string& data);

    std::string serialize() const;

  public:
    std::string sector;
    std::string spawnpoint;
    Vector position;
    bool is_checkpoint;
  };

public:
  GameSession(const std::string& levelfile, Savegame& savegame, Statistics* statistics = nullptr,
              bool preserve_music = false, const std::optional<SpawnPoint>& spawnpoint = std::nullopt,
              network::Host* host = nullptr);

  virtual void draw(Compositor& compositor) override;
  virtual void update(float dt_sec, const Controller& controller) override;
  virtual void setup() override;
  virtual void leave() override;
  virtual IntegrationStatus get_status() const override;

  void spawn_local_player(int id, const GameServerUser* target_user = nullptr, int target_id = -1);
  bool despawn_local_player(int id);
  std::pair<const GameServerUser*, int> spawn_remote_player(const GameServerUser& user, int id,
                                                            const GameServerUser* target_user = nullptr, int target_id = -1);
  bool despawn_remote_player(const GameServerUser& user, int id);

  /** ends the current level */
  void finish(bool win = true);
  void respawn(const std::string& sectorname, const std::string& spawnpointname,
               const GameServerUser* user = nullptr, bool all_users = true);
  void respawn_with_fade(const std::string& sectorname,
                         const std::string& spawnpointname,
                         const ScreenFade::FadeType fade_type,
                         const Vector &fade_point,
                         const bool make_invincible = false,
                         const GameServerUser* user = nullptr, bool all_users = true);
  void reset_level();

  void set_start_point(const std::string& sectorname,
                       const std::string& spawnpointname);
  void set_start_pos(const std::string& sectorname, const Vector& pos);
  void set_respawn_point(const std::string& sectorname,
                         const std::string& spawnpointname);
  void set_respawn_pos(const std::string& sectorname, const Vector& pos);
  void clear_respawn_points();

  const SpawnPoint& get_last_spawnpoint() const;

  void set_checkpoint_pos(const std::string& sectorname, const Vector& pos);
  const SpawnPoint* get_active_checkpoint_spawnpoint() const;

  Sector& get_current_sector() const { return *m_currentsector; }
  Level& get_current_level() const { return *m_level; }

  void start_sequence(Player* caller, Sequence seq, const SequenceData* data = nullptr);
  void set_target_timer_paused(bool paused);

  const std::string& get_level_file() const { return m_levelfile; }
  bool has_active_sequence() const;
  int restart_level(bool after_death = false, bool preserve_music = false);

  void toggle_pause();
  void abort_level();
  bool is_active() const;

  Savegame& get_savegame() const { return m_savegame; }

  void set_scheduler(SquirrelScheduler& new_scheduler);

private:
  void check_end_conditions();

  void drawstatus(DrawingContext& context);
  void draw_pause(DrawingContext& context);

  void on_escape_press(bool force_quick_respawn);

  Vector get_fade_point(const Vector& position = Vector(0, 0)) const;

public:
  bool reset_button;
  bool reset_checkpoint_button;

  bool m_prevent_death; /**< true if players should enter ghost mode instead of dying */

private:
  std::unique_ptr<Level> m_level;
  SurfacePtr m_statistics_backdrop;

  ssq::Table m_data_table;

  Sector* m_currentsector;

  bool  m_game_pause;
  float m_speed_before_pause;

  std::string m_levelfile;

  network::Host* m_network_host;

  // Spawnpoints
  std::vector<SpawnPoint> m_spawnpoints;
  const SpawnPoint* m_activated_checkpoint;

  struct SpawnRequest final
  {
    SpawnRequest();

    const GameServerUser* user; /**< the remote user whose players should be spawned. If nullptr, the request refers to local players. */
    bool all_users; /**< true if all local and remote players should be spawned. In that case, `user` is disregarded */

    std::string sector;
    std::string spawnpoint;
    ScreenFade::FadeType fade_type;
    std::unique_ptr<Timer> fade_timer;
    bool with_invincibility;
  };
  std::vector<SpawnRequest> m_spawn_requests;

  Statistics* m_best_level_statistics;
  Savegame& m_savegame;

  // Note: m_play_time should reset when a level is restarted from the beginning
  //       but NOT if Tux respawns at a checkpoint (for LevelTimes to work)
  float m_play_time; /**< total time in seconds that this session ran interactively */

  LevelIntro* m_levelintro;
  bool m_levelintro_shown; /**< true if the LevelIntro screen was already shown */

  bool m_active; /** Game active? **/

  bool m_end_seq_started;
  bool m_pause_target_timer;

  std::unique_ptr<GameObject> m_current_cutscene_text;

  Timer m_endsequence_timer;

private:
  GameSession(const GameSession&) = delete;
  GameSession& operator=(const GameSession&) = delete;
};

#endif

/* EOF */
