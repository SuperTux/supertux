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
#include "util/reader_fwd.hpp"

class TileSet;
class Sector;

/**
 * Represents a collection of Sectors running in a single GameSession.
 *
 * Each Sector in turn contains GameObjects, e.g. Badguys and Players.
 */
class Level
{
public:
  typedef std::vector<Sector*> Sectors;

  std::string name;
  std::string author;
  std::string contact;
  std::string license;
  std::string filename;
  std::string on_menukey_script;
  Sectors     sectors;
  Statistics  stats;
  float       target_time;
  TileSet    *tileset;
  bool        free_tileset;

public:
  Level();
  ~Level();

  // loads a levelfile
  void load(const std::string& filename);

  // saves to a levelfile
  void save(const std::string& filename);

  const std::string& get_name() const
  { return name; }

  const std::string& get_author() const
  { return author; }

  void add_sector(Sector* sector);

  Sector* get_sector(const std::string& name) const;

  size_t get_sector_count() const;
  Sector* get_sector(size_t num) const;

  const TileSet *get_tileset() const
  { return tileset; }

  int get_total_coins() const;
  int get_total_badguys() const;
  int get_total_secrets() const;

private:
  void load_old_format(const Reader& reader);

private:
  Level(const Level&);
  Level& operator=(const Level&);
};

#endif /*SUPERTUX_LEVEL_H*/

/* EOF */
