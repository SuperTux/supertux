//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//                2014 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_SUPERTUX_SUPERTUX_SAVEGAME_HPP
#define HEADER_SUPERTUX_SUPERTUX_SAVEGAME_HPP

#include <memory>
#include <string>
#include <vector>

#include "supertux/levelset.hpp"
#include "supertux/player_status.hpp"

struct LevelState
{
public:
  LevelState() :
    filename(),
    solved(false),
    perfect(false)
  {}

  std::string filename;
  bool solved;
  bool perfect;
};

struct LevelsetState
{
public:
  LevelsetState() :
    directory(),
    level_states()
  {}
  std::string directory;
  std::vector<LevelState> level_states;

  LevelState get_level_state(const std::string& filename) const;
  void store_level_state(const LevelState& state);
};

struct WorldmapState
{
public:
  WorldmapState() :
    filename(),
    level_states()
  {}
  std::string filename;
  std::vector<LevelState> level_states;
};

class Savegame final
{
public:
  struct Progress
  {
  public:
    Progress() :
      solved(),
      total()
    {}
    Progress(int solved_, int total_) :
      solved(solved_),
      total(total_)
    {}
    int solved;
    int total;

    int get_percentage() const;
  };

public:
  static std::unique_ptr<Savegame> from_file(const std::string& filename);

public:
  Savegame(const std::string& filename);

  /** Returns content of (tux ...) entry */
  PlayerStatus& get_player_status() const { return *m_player_status; }

  std::string get_title() const;
  const std::string& get_filename() const { return m_filename; }

  std::vector<std::string> get_levelsets() const;
  LevelsetState get_levelset_state(const std::string& name) const;
  void set_levelset_state(const std::string& basedir,
                          const std::string& level_filename,
                          bool solved);

  Progress get_levelset_progress() const;
  Progress get_worldmap_progress() const;

  std::vector<std::string> get_worldmaps() const;
  WorldmapState get_worldmap_state(const std::string& name) const;

  void save();

  bool is_title_screen() const;

private:
  void load();
  void clear_state_table();

private:
  std::string m_filename;
  std::unique_ptr<PlayerStatus> m_player_status;

private:
  Savegame(const Savegame&) = delete;
  Savegame& operator=(const Savegame&) = delete;
};

#endif

/* EOF */
