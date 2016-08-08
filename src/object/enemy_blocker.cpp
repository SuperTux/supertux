//  SuperTux
//  Copyright (C) 2007 Kovago Zoltan <DirtY.iCE.hu@gmail.com>
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

#include "object/enemy_blocker.hpp"

#include "editor/editor.hpp"
#include "badguy/badguy.hpp"
#include "supertux/object_factory.hpp"
#include "util/gettext.hpp"
#include "util/reader_mapping.hpp"
#include "video/drawing_context.hpp"

EnemyBlocker::EnemyBlocker(const ReaderMapping& lisp):
  width(),
  height()
{
  if (!lisp.get("name" , name)) name = "";
  if (!lisp.get("x", bbox.p1.x)) bbox.p1.x = 0;
  if (!lisp.get("y", bbox.p1.y)) bbox.p1.y = 0;
  if (!lisp.get("width", width)) width = 32;
  if (!lisp.get("height", height)) height = 32;

  bbox.set_size(width, height);
}

ObjectSettings
EnemyBlocker::get_settings() {
  width = bbox.get_width();
  height = bbox.get_height();

  ObjectSettings result = MovingObject::get_settings();
  result.options.push_back( ObjectOption(MN_NUMFIELD, _("Width"), &width, "width"));
  result.options.push_back( ObjectOption(MN_NUMFIELD, _("Height"), &height, "height"));

  return result;
}

void
EnemyBlocker::after_editor_set() {
  bbox.set_size(width, height);
}

bool
EnemyBlocker::collides(GameObject& other, const CollisionHit& )
{
  BadGuy* badguy = dynamic_cast<BadGuy*> (&other);
  
  if (badguy == 0)
    return ABORT_MOVE;
  return FORCE_MOVE;
}

HitResponse
EnemyBlocker::collision(GameObject& other, const CollisionHit& )
{
  BadGuy* badguy = dynamic_cast<BadGuy*> (&other);
  
  if (badguy == 0)
    return ABORT_MOVE;
  return FORCE_MOVE;
}

void
EnemyBlocker::draw(DrawingContext& context)
{
  if (Editor::is_active()) {
    context.draw_filled_rect(bbox, Color(0.96f, 0.38f, 0.0f, 0.8f),
                             0.0f, LAYER_OBJECTS);
  }
}

void
EnemyBlocker::update(float )
{
}

/* EOF */
