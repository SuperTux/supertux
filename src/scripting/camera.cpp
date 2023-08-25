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
#include "supertux/sector.hpp"
#include "util/dynamic_scoped_ref.hpp"
#include "util/log.hpp"

namespace scripting {

void
Camera::reload_config()
{
  SCRIPT_GUARD_VOID;
  BIND_SECTOR(::Sector::get());
  object.reload_config();
}

void
Camera::shake(float speed, float x, float y)
{
  SCRIPT_GUARD_VOID;
  BIND_SECTOR(::Sector::get());
  object.shake(speed, x, y);
}

void
Camera::set_pos(float x, float y)
{
  SCRIPT_GUARD_VOID;
  BIND_SECTOR(::Sector::get());
  object.scroll_to(Vector(x, y), 0.0f);
}

void
Camera::move(float x, float y)
{
  SCRIPT_GUARD_VOID;
  BIND_SECTOR(::Sector::get());
  object.scroll_to(Vector(object.get_position().x + x,object.get_position().y + y), 0.0f);
}

void
Camera::set_mode(const std::string& mode)
{
  SCRIPT_GUARD_VOID;
  BIND_SECTOR(::Sector::get());

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
  BIND_SECTOR(::Sector::get());
  object.scroll_to(Vector(x, y), scrolltime);
}

float
Camera::get_current_scale()
{
  SCRIPT_GUARD_DEFAULT;
  BIND_SECTOR(::Sector::get());
  return object.get_current_scale();
}

float
Camera::get_target_scale()
{
  SCRIPT_GUARD_DEFAULT;
  BIND_SECTOR(::Sector::get());
  return object.get_target_scale();
}

void
Camera::set_scale(float scale)
{
  ease_scale(scale, 0, "");
}

void
Camera::set_scale_anchor(float scale, int anchor)
{
  ease_scale_anchor(scale, 0, anchor, "");
}

void
Camera::scale(float scale, float time)
{
  ease_scale(scale, time, "");
}

void
Camera::scale_anchor(float scale, float time, int anchor)
{
  ease_scale_anchor(scale, time, anchor, "");
}

void
Camera::ease_scale(float scale, float time, const std::string& ease)
{
  SCRIPT_GUARD_VOID;
  BIND_SECTOR(::Sector::get());
  object.ease_scale(scale, time, getEasingByName(EasingMode_from_string(ease)));
}

void
Camera::ease_scale_anchor(float scale, float time, int anchor, const std::string& ease)
{
  SCRIPT_GUARD_VOID;
  BIND_SECTOR(::Sector::get());
  object.ease_scale(scale, time, static_cast<AnchorPoint>(anchor), getEasingByName(EasingMode_from_string(ease)));
}

int
Camera::get_screen_width()
{
  SCRIPT_GUARD_DEFAULT;
  BIND_SECTOR(::Sector::get());
  return object.get_screen_size().width;
}

int
Camera::get_screen_height()
{
  SCRIPT_GUARD_DEFAULT;
  BIND_SECTOR(::Sector::get());
  return object.get_screen_size().height;
}

float
Camera::get_x()
{
  SCRIPT_GUARD_DEFAULT;
  BIND_SECTOR(::Sector::get());
  return object.get_position().x;
}

float
Camera::get_y()
{
  SCRIPT_GUARD_DEFAULT;
  BIND_SECTOR(::Sector::get());
  return object.get_position().y;
}
} // namespace scripting

/* EOF */
