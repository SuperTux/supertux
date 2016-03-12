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

#ifndef HEADER_SUPERTUX_EDITOR_OBJECT_GROUP_HPP
#define HEADER_SUPERTUX_EDITOR_OBJECT_GROUP_HPP

#include <string>
#include <vector>

#include "editor/object_icon.hpp"

#include "util/reader_fwd.hpp"

class ObjectGroup
{
  public:
    ObjectGroup();
    ObjectGroup(const Reader& reader);
    ~ObjectGroup();

    std::string name;
    std::vector<ObjectIcon> icons;

    void add_icon(std::string object, std::string icon_path);
};

#endif // HEADER_SUPERTUX_EDITOR_OBJECT_GROUP_HPP

/* EOF */
