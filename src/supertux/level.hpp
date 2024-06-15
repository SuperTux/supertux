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

#ifndef HEADER_SUPERTUX_SUPERTUX_LEVEL_HPP
#define HEADER_SUPERTUX_SUPERTUX_LEVEL_HPP

#include "supertux/statistics.hpp"

class Player;
class ReaderMapping;
class Sector;
class Writer;

/** Represents a collection of Sectors running in a single GameSession.

    Each Sector in turn contains GameObjects, e.g. Badguys and Players. */
class Level final
{
  friend class LevelParser;

public:
  static Level* current() { return s_current; }

private:
  static Level* s_current;

public:
  explicit Level(bool m_is_worldmap);
  ~Level();

  // saves to a levelfile
  void save(const std::string& filename, bool retry = false);
  void save(std::ostream& stream);

  void add_sector(std::unique_ptr<Sector> sector);
  const std::string& get_name() const { return m_name; }
  const std::string& get_author() const { return m_author; }

  Sector* get_sector(const std::string& name) const;

  size_t get_sector_count() const;
  Sector* get_sector(size_t num) const;
  const std::vector<std::unique_ptr<Sector> >& get_sectors() const { return m_sectors; }

  std::vector<Player*> get_players() const;

  const std::string& get_tileset() const { return m_tileset; }

  int get_total_coins() const;
  int get_total_badguys() const;
  int get_total_secrets() const;

  void reactivate();

  bool is_worldmap() const { return m_is_worldmap; }

  const std::string& get_license() const { return m_license; }

private:
  void initialize();

  void save(Writer& writer);
  void load_old_format(const ReaderMapping& reader);

public:
  bool m_is_worldmap;
  std::string m_name;
  std::string m_author;
  std::string m_contact;
  std::string m_license;
  std::string m_filename;
  std::string m_note;
  std::vector<std::unique_ptr<Sector> > m_sectors;
  Statistics m_stats;
  float m_target_time;
  std::string m_tileset;
  bool m_suppress_pause_menu;
  bool m_is_in_cutscene;
  bool m_skip_cutscene;
  std::string m_icon;
  std::string m_icon_locked;
  std::string m_wmselect_bkg;

private:
  Level(const Level&) = delete;
  Level& operator=(const Level&) = delete;
};

#endif

/* EOF */
