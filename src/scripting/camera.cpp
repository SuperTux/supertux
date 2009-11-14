//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include <config.h>

#include <string>
#include <stdio.h>
#include "object/camera.hpp"
#include "scripting/camera.hpp"
#include "math/vector.hpp"

#define NOIMPL      log_fatal << __FUNCTION__ << " not implemented."

namespace Scripting
{
  Camera::Camera(::Camera* camera)
    : camera(camera)
  { }

  Camera::~Camera()
  { }

  void
  Camera::reload_config()
  {
    camera->reload_config();
  }

  void
  Camera::shake(float speed, float x, float y)
  {
    camera->shake(speed, x, y);
  }

  void
  Camera::set_pos(float , float )
  {
  }

  void
  Camera::set_mode(const std::string& mode)
  {
    if(mode == "normal") {
      camera->mode = ::Camera::NORMAL;
    } else if(mode == "manual") {
      camera->mode = ::Camera::MANUAL;
    } else {
      log_fatal << "Camera mode '" << mode << "' unknown.";
    }
  }

  void
  Camera::scroll_to(float x, float y, float scrolltime)
  {
    camera->scroll_to(Vector(x, y), scrolltime);
  }
}
