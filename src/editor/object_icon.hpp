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

#ifndef HEADER_SUPERTUX_EDITOR_OBJECT_ICON_HPP
#define HEADER_SUPERTUX_EDITOR_OBJECT_ICON_HPP

#include "util/reader_fwd.hpp"
#include "util/reader_mapping.hpp"
#include "video/surface.hpp"
#include "video/drawing_context.hpp"

class ObjectIcon
{
  public:
    ObjectIcon(const std::string& name, const std::string& icon);
    ObjectIcon(const ReaderMapping& reader);
    virtual ~ObjectIcon();

    std::string object_name;
    SurfacePtr surface;

    virtual void draw(DrawingContext& context, Vector pos);

  private:
    Vector offset;

    void calculate_offset();
};

#endif // HEADER_SUPERTUX_EDITOR_OBJECT_ICON_HPP

/* EOF */
