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

#ifndef SUPERTUX_SPRITE_H
#define SUPERTUX_SPRITE_H

#include <string>
#include <assert.h>
#include <SDL.h>

#include "video/surface.hpp"
#include "math/vector.hpp"
#include "sprite_data.hpp"

class DrawingContext;

class Sprite
{
public:
  Sprite(SpriteData& data);
  Sprite(const Sprite& other);
  ~Sprite();

  /** Draw sprite, automatically calculates next frame */
  void draw(DrawingContext& context, const Vector& pos, int layer);

  void draw_part(DrawingContext& context, const Vector& source,
      const Vector& size, const Vector& pos, int layer);

  /** Set action (or state) */
  void set_action(const std::string& act, int loops = -1);

  /* Stop animation */
  void stop_animation()
  { animation_loops = 0; }
  /** Check if animation is stopped or not */
  bool check_animation();

  float get_fps() const
  { return action->fps; }
  /** Get current action total frames */
  int get_frames() const
  { return action->surfaces.size(); }
  /** Get sprite's name */
  const std::string& get_name() const
  { return data.name; }
  /** Get current action name */
  const std::string& get_action_name() const
  { return action->name; }

  int get_width() const;
  int get_height() const;

  /** Get current frame */
  int get_frame() const
  { return (int)frame; }
  /** Set current frame */
  void set_frame(int frame_)
  { if(frame_ > get_frames()) frame = 0; else frame = frame_; }
  Surface* get_frame(unsigned int frame)
  {
    assert(frame < action->surfaces.size());
    return action->surfaces[frame];
  }    

private:
  void update();

  SpriteData& data;

  float frame;
  int animation_loops;
  Uint32 last_ticks;

  SpriteData::Action* action;
};

#endif

