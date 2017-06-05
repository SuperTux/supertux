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

#include "object/invisible_wall.hpp"

#include "editor/editor.hpp"
#include "util/gettext.hpp"
#include "util/reader_mapping.hpp"
#include "video/drawing_context.hpp"

InvisibleWall::InvisibleWall(const ReaderMapping& lisp):
  width(),
  height()
{
  lisp.get("name" , name, "");
  lisp.get("x", bbox.p1.x, 0);
  lisp.get("y", bbox.p1.y, 0);
  lisp.get("width", width, 32);
  lisp.get("height", height, 32);

  bbox.set_size(width, height);

  group = COLGROUP_STATIC;
}

ObjectSettings
InvisibleWall::get_settings() {
  width = bbox.get_width();
  height = bbox.get_height();

  ObjectSettings result = MovingObject::get_settings();
  result.options.push_back( ObjectOption(MN_NUMFIELD, _("Width"), &width, "width"));
  result.options.push_back( ObjectOption(MN_NUMFIELD, _("Height"), &height, "height"));

  return result;
}

void
InvisibleWall::after_editor_set() {
  bbox.set_size(width, height);
}

HitResponse
InvisibleWall::collision(GameObject& , const CollisionHit& )
{
  return FORCE_MOVE;
}

void
InvisibleWall::draw(DrawingContext& context)
{
  if (Editor::is_active()) {
    context.draw_filled_rect(bbox, Color(0.0f, 0.0f, 0.0f, 0.6f),
                             0.0f, LAYER_OBJECTS);
  }
}

void
InvisibleWall::update(float )
{
}

/* EOF */
