//  $Id$
//
//  SuperTux -  A Jump'n Run
//  Copyright (C) 2004 Matthias Braun <matze@braunis.de
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
#ifndef SUPERTUX_CAMERA_H
#define SUPERTUX_CAMERA_H

#include <vector>
#include <cassert>

#include "math/vector.h"
#include "game_object.h"
#include "video/drawing_context.h"
#include "serializable.h"
#include "timer.h"

namespace lisp {
class Lisp;
}

class Sector;

class Camera : public GameObject, public Serializable
{
public:
  Camera(Sector* sector);
  virtual ~Camera();

  /// parse camera mode from lisp file
  void parse(const lisp::Lisp& reader);
  /// write camera mode to a lisp file
  virtual void write(lisp::Writer& writer);

  /// reset camera postion
  virtual void reset(const Vector& tuxpos);

  /** @deprecated@ */
  const Vector& get_translation() const;

  virtual void action(float elapsed_time);

  virtual void draw(DrawingContext& )
  {
  }

  // shake camera in a direction 1 time
  void shake(float speed, float x, float y);

  void set_scrolling(int scroll_x, int scroll_y)
  {
    translation.x = scroll_x;
    translation.y = scroll_y;
  }

  enum CameraMode
  {
    NORMAL, AUTOSCROLL, MANUAL
  };
  CameraMode mode;

private:
  void scroll_normal(float elapsed_time);
  void scroll_autoscroll(float elapsed_time);
  void keep_in_bounds();
  void shake();

  enum LeftRightScrollChange
  {
    NONE, LEFT, RIGHT
  };
    
  Vector translation;

  Sector* sector;

  // normal mode
  bool do_backscrolling;
  LeftRightScrollChange scrollchange;

  // autoscroll mode
  class ScrollPoint {
  public:
    Vector position;
    float speed;
  };
  std::vector<ScrollPoint> scrollpoints;
  size_t auto_idx;
  float auto_t;
  Vector current_dir;

  // shaking
  Timer2 shaketimer;
  float shakespeed;
  float shakedepth_x;
  float shakedepth_y;
};

#endif /*SUPERTUX_CAMERA_H*/

