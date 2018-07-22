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

#include "editor/spawnpoint_marker.hpp"

#include "supertux/spawn_point.hpp"
#include "util/reader_mapping.hpp"
#include "video/drawing_context.hpp"
#include "video/surface.hpp"

SpawnPointMarker::SpawnPointMarker (const ReaderMapping& lisp) :
  surface(Surface::create("images/engine/editor/spawnpoint.png"))
{
  lisp.get("name", name, "");
  lisp.get("x", bbox.p1.x, 0);
  lisp.get("y", bbox.p1.y, 0);

  setup();
}

SpawnPointMarker::SpawnPointMarker (const SpawnPoint* sp) :
  surface(Surface::create("images/engine/editor/spawnpoint.png"))
{
  name = sp->name;
  bbox.p1 = sp->pos;
  setup();
}

void SpawnPointMarker::setup() {
  bbox.set_size(32, 32);
}

void SpawnPointMarker::draw(DrawingContext& context) {
  context.color().draw_surface(surface, bbox.p1, LAYER_FOREGROUND1);
}

/* EOF */
