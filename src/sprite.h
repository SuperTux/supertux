//  $Id$
// 
//  SuperTux
//  Copyright (C) 2004 Ingo Ruhnke <grumbel@gmx.de>
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

#ifndef HEADER_SPRITE_HXX
#define HEADER_SPRITE_HXX

#include <string>
#include <vector>
#include "lispreader.h"
#include "texture.h"

class Sprite
{
 private:
  std::string name;

  int x_hotspot;
  int y_hotspot;

  /** Frames per second */
  float fps;

  /** Number of seconds that a frame is displayed until it is switched
      to the next frame */
  float frame_delay;

  float time;

  std::vector<Surface*> surfaces;

  void init_defaults();
 public:
  /** cur has to be a pointer to data in the form of ((x-hotspot 5)
      (y-hotspot 10) ...) */
  Sprite(lisp_object_t* cur);
  ~Sprite();
  
  void reset();

  /** Update the sprite and process to the next frame */
  void update(float delta);
  void draw(float x, float y);
  void draw_part(float sx, float sy, float x, float y, float w, float h);
  int get_current_frame() const;

  std::string get_name() const { return name; } 
  int get_width() const;
  int get_height() const;
};

#endif

/* Local Variables: */
/* mode:c++ */
/* End: */
