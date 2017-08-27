//  SuperTux
//  Copyright (C) 2015 Hume2 <teratux.mail@gmail.com>
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

#ifndef HEADER_SUPERTUX_EDITOR_OBJECT_INPUT_HPP
#define HEADER_SUPERTUX_EDITOR_OBJECT_INPUT_HPP

#include "editor/object_group.hpp"

class ObjectInput
{
private:
  int get_num_groups(bool for_worldmap) const
  {
    int num_groups = 0;
    for(const auto& group : groups)
    {
      if(group.for_worldmap == for_worldmap)
      {
        num_groups++;
      }
    }
    return num_groups;
  }

  public:
    ObjectInput();
    ~ObjectInput();

    std::vector<ObjectGroup> groups;
    int get_num_worldmap_groups() const
    {
      return get_num_groups(true);
    }
    int get_num_level_groups() const
    {
      return get_num_groups(false);
    }
    int get_first_worldmap_group_index() const
    {
      int worldmap_group_index = 0;
      for(const auto& group : groups)
      {
        if(group.for_worldmap)
        {
          return worldmap_group_index;
        }
        worldmap_group_index++;
      }
      return -1;
    }
};

#endif // HEADER_SUPERTUX_EDITOR_OBJECT_INPUT_HPP

/* EOF */
