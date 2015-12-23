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

#include "supertux/globals.hpp"
#include "supertux/resources.hpp"
#include "util/gettext.hpp"
#include "video/renderer.hpp"
#include "video/video_system.hpp"

SpawnPointMarker::SpawnPointMarker (const ReaderMapping& lisp) :
  surface()
{
  if ( !lisp.get("name", name)) name = "";
  if ( !lisp.get("x", bbox.p1.x)) bbox.p1.x = 0;
  if ( !lisp.get("y", bbox.p1.y)) bbox.p1.y = 0;

  bbox.set_size(32, 32);

  surface = Surface::create("images/engine/editor/spawnpoint.png");
}

SpawnPointMarker::~SpawnPointMarker() {

}

void SpawnPointMarker::draw(DrawingContext& context) {
  context.draw_surface(surface, bbox.p1, LAYER_FOREGROUND1);
}

ObjectSettings
SpawnPointMarker::get_settings() {
  ObjectSettings result(_("Spawn Point"));
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Name"), &name));
  return result;
}

/* EOF */
