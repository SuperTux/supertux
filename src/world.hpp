//  $Id: worldmap.hpp 2800 2005-10-02 22:57:31Z matzebraun $
// 
//  SuperTux
//  Copyright (C) 2004 Ingo Ruhnke <grumbel@gmx.de>
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#ifndef SUPERTUX_WORLD_H
#define SUPERTUX_WORLD_H

#include <vector>
#include <string>

class World
{
private:
  std::vector<std::string> levels;
  std::string basedir;

public:
  World();
  ~World();

  void load(const std::string& filename);
  
  const std::string& get_level_filename(unsigned int i) const;
  unsigned int get_num_levels() const;

  const std::string& get_basedir() const;

  void run();

  std::string title;
  std::string description;
  bool hide_from_contribs;
  bool is_levelset;
};

#endif

