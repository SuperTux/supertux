//  $Id$
// 
//  SuperTux
//  Copyright (C) 2004 SuperTux Development Team, see AUTHORS for details
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

#include <map>
#include <string>

class Sector;
class LispReader;

class Level
{
public:
  std::string name;
  std::string author;
  int time_left;
  typedef std::map<std::string, Sector*> Sectors;
  Sectors sectors;

public:
  Level();
  ~Level();

  void load(const std::string& filename);
  void save(const std::string& filename);

  const std::string& get_name() const
  { return name; }

  const std::string& get_author() const
  { return author; }

  /** Flips the level vertically */
  void do_vertical_flip();

  void add_sector(Sector* sector);

  Sector* get_sector(const std::string& name);

private:
  void load_old_format(LispReader& reader);
};

#endif /*SUPERTUX_LEVEL_H*/
