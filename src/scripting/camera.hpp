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

#ifndef __CAMERA_H__
#define __CAMERA_H__

#ifndef SCRIPTING_API
class Camera;
typedef Camera _Camera;
#include <string>
#endif

namespace Scripting
{

class Camera
{
public:
#ifndef SCRIPTING_API
  Camera(_Camera* camera);
  ~Camera();
#endif

  void reload_config();

  /** Shake the camera */
  void shake(float speed, float x, float y);
  /** Set camera to a specific coordinate */
  void set_pos(float x, float y);
  /** Set camera to a specific mode, can be "normal", "manual" */
  void set_mode(const std::string& mode);
  /** Scroll camera to position x,y in scrolltime seconds */
  void scroll_to(float x, float y, float scrolltime);

#ifndef SCRIPTING_API
  _Camera* camera;
#endif
};

}

#endif
