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

#include "supertux/object_factory.hpp"
#include "util/editor_active.hpp"
#include "util/reader_mapping.hpp"

InvisibleWall::InvisibleWall(const ReaderMapping& lisp) :
  MovingSprite(lisp, "images/objects/invisible/invisible.sprite", LAYER_TILES, COLGROUP_STATIC),
  physic(),
  width(),
  height()
{
  if (!lisp.get("width", width)) width = 32;
  if (!lisp.get("height", height)) height = 32;
  bbox.set_size(width, height);
}

void
InvisibleWall::save(Writer& writer) {
  MovingSprite::save(writer);
  writer.write("width", width);
  writer.write("height", height);
}

ObjectSettings
InvisibleWall::get_settings() {
  ObjectSettings result = MovingSprite::get_settings();
  result.options.push_back( ObjectOption(MN_NUMFIELD, _("Width"), &width));
  result.options.push_back( ObjectOption(MN_NUMFIELD, _("Height"), &height));

  return result;
}

HitResponse
InvisibleWall::collision(GameObject& , const CollisionHit& )
{
  return FORCE_MOVE;
}

void
InvisibleWall::draw(DrawingContext& context)
{
  if (EditorActive()) {
    context.draw_filled_rect(bbox, Color(0.0f, 0.0f, 0.0f, 0.6f),
                             0.0f, LAYER_OBJECTS);
  }
}

/* EOF */
