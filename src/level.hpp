//  $Id$
//
//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.

#ifndef SUPERTUX_LEVEL_H
#define SUPERTUX_LEVEL_H

#include <vector>
#include <string>
#include "statistics.hpp"
#include "sector.hpp"

namespace lisp {
class Lisp;
}

/**
 * Represents a collection of Sectors running in a single GameSession.
 *
 * Each Sector in turn contains GameObjects, e.g. Badguys and Players.
 */
class Level
{
public:
  std::string name;
  std::string author;
  std::string contact;
  std::string license;
  std::string on_menukey_script;
  typedef std::vector<Sector*> Sectors;
  Sectors sectors;
  Statistics stats;

public:
  Level();
  ~Level();

  // loads a levelfile
  void load(const std::string& filename);
  void save(const std::string& filename);

  const std::string& get_name() const
  { return name; }

  const std::string& get_author() const
  { return author; }

  void add_sector(Sector* sector);

  Sector* get_sector(const std::string& name);

  size_t get_sector_count();
  Sector* get_sector(size_t num);

  int get_total_coins();
  int get_total_badguys();

  /** Get total number of GameObjects of given type */
  template<class T> int get_total_count()
  {
    int total = 0;
    for(Sectors::iterator i = sectors.begin(); i != sectors.end(); ++i) {
      total += (*i)->get_total_count<T>();
    }
    return total;
  }

private:
  void load_old_format(const lisp::Lisp& reader);
};

#endif /*SUPERTUX_LEVEL_H*/
