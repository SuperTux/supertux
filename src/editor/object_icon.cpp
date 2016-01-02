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

#include <string>

#include "editor/object_icon.hpp"
#include "math/rect.hpp"
#include "supertux/globals.hpp"
#include "supertux/resources.hpp"
#include "video/renderer.hpp"
#include "video/video_system.hpp"
#include "util/reader.hpp"
#include "util/reader_mapping.hpp"

ObjectIcon::ObjectIcon(std::string name, std::string icon) :
  object_name(name),
  surface()
{
  surface = Surface::create(icon);
}

ObjectIcon::ObjectIcon(const ReaderMapping& reader) :
  object_name(),
  surface()
{
  std::string icon = "images/engine/icons/supertux.png";
  reader.get("class", object_name);
  reader.get("icon", icon);
  surface = Surface::create(icon);
}

ObjectIcon::~ObjectIcon() {

}

void
ObjectIcon::draw(DrawingContext& context, Vector pos) {
  context.draw_surface_part(surface, Rectf(Vector(0,0), surface->get_size()),
                            Rectf(pos, pos + Vector(32,32)), LAYER_GUI - 9);
}

/* EOF */
