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

#ifndef SUPERTUX_LEVEL_SUBSET_H
#define SUPERTUX_LEVEL_SUBSET_H

#include <vector>
#include <string>
#include "utils/lispreader.h"

using namespace SuperTux;

namespace SuperTux {
class Surface;
};

/** This type holds meta-information about a level-subset. 
    It could be extended to handle manipulation of subsets. */
class LevelSubset
{
private:
  /** Directory in which the level subset is stored */
  std::string directory;

  /** Level filenames without the leading path ("level1.stl",
      "level3.stl", ...) */
  std::vector<std::string> levels;

public:
  LevelSubset();
  ~LevelSubset();

  static void create(const std::string& subset_name);
  void load(const std::string& filename);
  void save();

  void add_level(const std::string& name);

  std::string get_level_filename(unsigned int i);
  int get_num_levels() const;

  std::string name;
  std::string title;
  std::string description;
  bool hide_from_contribs;

private:
  void read_info_file(const std::string& info_file);
};

#endif

/* Local Variables: */
/* mode:c++ */
/* End: */
