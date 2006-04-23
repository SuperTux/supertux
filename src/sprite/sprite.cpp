//  $Id$
//
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

#include <iostream>
#include <cmath>
#include <cassert>
#include <stdexcept>

#include "sprite.hpp"
#include "video/drawing_context.hpp"
#include "log.hpp"
#include "timer.hpp"

Sprite::Sprite(SpriteData& newdata)
  : data(newdata), frame(0), animation_loops(-1)
{
  action = data.get_action("normal");
  if(!action)
    action = data.actions.begin()->second;
  last_ticks = real_time;
}

Sprite::Sprite(const Sprite& other)
  : data(other.data), frame(other.frame),
    animation_loops(other.animation_loops),
    action(other.action)
{
  last_ticks = real_time;
}

Sprite::~Sprite()
{
}

void
Sprite::set_action(const std::string& name, int loops)
{
  if(action && action->name == name)
    return;

  SpriteData::Action* newaction = data.get_action(name);
  if(!newaction) {
    log_debug << "Action '" << name << "' not found." << std::endl;
    return;
  }

  action = newaction;
  animation_loops = loops;
  frame = 0;
}

bool
Sprite::animation_done()
{
  return animation_loops == 0;
}

void
Sprite::update()
{
  if(animation_done())
    return;

  float frame_inc = action->fps * (real_time - last_ticks);
  last_ticks = real_time;

  frame += frame_inc;

  if(frame >= get_frames()) {
    frame = fmodf(frame, get_frames());
      
    animation_loops--;
    if(animation_done())
      frame = get_frames()-1;
  }
}

void
Sprite::draw(DrawingContext& context, const Vector& pos, int layer)
{
  assert(action != 0);
  update();

  if((int)frame >= get_frames() || (int)frame < 0)
    log_warning << "frame out of range: " << (int)frame << "/" << get_frames() << " at " << get_name() << "/" << get_action_name() << std::endl;
  else
    context.draw_surface(action->surfaces[(int)frame],
            pos - Vector(action->x_offset, action->y_offset),
            layer + action->z_order);
}

void
Sprite::draw_part(DrawingContext& context, const Vector& source,
    const Vector& size, const Vector& pos, int layer)
{
  assert(action != 0);
  update();

  int frameidx = (int) frame;
  
  if(frameidx >= get_frames() || frameidx < 0) {
#ifndef DEBUG
    // in optimized mode we get some small rounding errors in floating point
    // number sometimes...
    log_warning << "frame out of range: " << frameidx << "/" << get_frames() << " at sprite: " << get_name() << "/" << get_action_name() << std::endl;
#endif
    frameidx = get_frames() - 1;
  }
    
  context.draw_surface_part(action->surfaces[frameidx], source, size,
      pos - Vector(action->x_offset, action->y_offset),
      layer + action->z_order);
}

int
Sprite::get_width() const
{
  return (int) action->surfaces[get_frame()]->get_width();
}

int
Sprite::get_height() const
{
  return (int) action->surfaces[get_frame()]->get_height();
}

void
Sprite::set_fps(float new_fps)
{
  action->fps = new_fps;
}


