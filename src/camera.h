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
#ifndef __VIEWPORT_H__
#define __VIEWPORT_H__

#include "vector.h"
#include "game_object.h"
#include "serializable.h"

class LispReader;
class Player;
class Level;

class Camera : public GameObject, public Serializable
{
public:
  Camera(Player* player = 0, Level* level = 0);
  virtual ~Camera();

  /** transforms a coordinate in world space to screen space.
   * Basically you have to apply this function to each coordinate that you want
   * to display on screen.
   */
  Vector world2screen(const Vector& worldpos) const
  {
    return worldpos - translation;                   
  }                                                  

  /// parse camera mode from lisp file
  void parse_camera(LispReader& reader);
  /// write camera mode to a lisp file
  virtual void write(LispWriter& writer);

  /** returns the current translation (=scroll) vector of the viewport */
  const Vector& get_translation() const
  { return translation; }
  /** set the curren translation vector of the viewport */
  void set_translation(const Vector& translation);

  virtual void action(float elapsed_time);

  enum CameraMode
  {
    NORMAL, AUTOSCROLL, MANUAL
  };

private:
  void scroll_normal(float elapsed_time);
  void scroll_autoscroll(float elapsed_time);

  enum LeftRightScrollChange
  {
    NONE, LEFT, RIGHT
  };
    
  Vector translation;

  Player* player;
  Level* level;
  CameraMode mode;
  LeftRightScrollChange scrollchange;
};

#endif

