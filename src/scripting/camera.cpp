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

#include "object/camera.hpp"

#include "scripting/camera.hpp"
#include "util/log.hpp"

namespace scripting {

void
Camera::reload_config()
{
  SCRIPT_GUARD_VOID;
  object.reload_config();
}

void
Camera::shake(float speed, float x, float y)
{
  SCRIPT_GUARD_VOID;
  object.shake(speed, x, y);
}

void
Camera::set_pos(float , float )
{
}

void
Camera::set_mode(const std::string& mode)
{
  SCRIPT_GUARD_VOID;

  if (mode == "normal") {
    object.set_mode(::Camera::Mode::NORMAL);
  } else if (mode == "manual") {
    object.set_mode(::Camera::Mode::MANUAL);
  } else {
    log_fatal << "Camera mode '" << mode << "' unknown.";
  }
}

void
Camera::scroll_to(float x, float y, float scrolltime)
{
  SCRIPT_GUARD_VOID;
  object.scroll_to(Vector(x, y), scrolltime);
}

} // namespace scripting

/* EOF */
