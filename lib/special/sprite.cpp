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
#include <config.h>

#include <iostream>
#include <cmath>
#include <stdexcept>

#include "../app/globals.h"
#include "../app/setup.h"
#include "../special/sprite.h"
#include "../video/drawing_context.h"

namespace SuperTux
{

Sprite::Sprite(SpriteData& newdata)
  : data(newdata)
{
  action = data.actions.begin()->second;
  reset();
}

Sprite::Sprite(const Sprite& other)
  : data(other.data), frame(other.frame),
    animation_loops(other.animation_loops), last_tick(other.last_tick),
    action(other.action), next_action(other.next_action)
{
}

Sprite::~Sprite()
{
}

void
Sprite::set_action(std::string name)
{
  if(!next_action.empty() && animation_loops > 0) {
    next_action = name;
    return;
  }
  SpriteData::Action* newaction = data.get_action(name);
  if(!action)
    return;

  action = newaction;
}

void
Sprite::start_animation(int loops)
{
  reset();
  animation_loops = loops;
}

void
Sprite::reset()
{
  frame = 0;
  last_tick = SDL_GetTicks();
  animation_reversed = false;
  animation_loops = -1;
  next_action.clear();
}

bool
Sprite::check_animation()
{
  return animation_loops;
}

void
Sprite::reverse_animation(bool reverse)
{
  animation_reversed = reverse;

  if(animation_reversed)
    frame = get_frames()-1;
  else
    frame = 0;
}

void
Sprite::update()
{
  if(animation_loops == 0)
  {
    if(frame >= get_frames() || frame < 0)
      frame = 0;
    return;
  }

  float frame_inc = (action->fps/1000.0) * (SDL_GetTicks() - last_tick);
  last_tick = SDL_GetTicks();

  if(animation_reversed)
    frame -= frame_inc;
  else
    frame += frame_inc;

  if(animation_reversed) {
    if(frame < 0 || frame >= (float)get_frames()) {
      // last case can happen when not used reverse_animation()
      float excedent = frame - 0;
      frame = get_frames() - 1;
      if(animation_loops > 0)
      {
        animation_loops--;
        if(animation_loops == 0 && !next_action.empty())
        {
          set_action(next_action);
          start_animation(-1);
        }
      }

      if(fabsf(excedent) < get_frames())
        frame += excedent;
    }
  }
  else
  {
    if(frame >= (float)get_frames())
    {
      float excedent = frame - get_frames();
      frame = 0;
      if(animation_loops > 0)
      {
        animation_loops--;
        if(animation_loops == 0 && !next_action.empty())
        {
          set_action(next_action);
          start_animation(-1);
        }
      }

      if(excedent < get_frames())
        frame += excedent;
    }
  }
}

void
Sprite::draw(DrawingContext& context, const Vector& pos, int layer,
    Uint32 drawing_effect)
{
  update();

  if((int)frame >= get_frames() || (int)frame < 0)
    std::cerr << "Warning: frame out of range: " << (int)frame
              << "/" << get_frames() << " at " << get_name()
              << "/" << get_action_name() << std::endl;
  else
    context.draw_surface(action->surfaces[(int)frame],
            pos - Vector(action->x_offset, action->y_offset),
            layer + action->z_order, drawing_effect);
}

void
Sprite::draw_part(DrawingContext& context, const Vector& source,
    const Vector& size, const Vector& pos, int layer, Uint32 drawing_effect)
{
  update();

  if((int)frame >= get_frames() || (int)frame < 0)
    std::cerr << "Warning: frame out of range: " << (int)frame
              << "/" << get_frames() << " at sprite: " << get_name()
              << "/" << get_action_name() << std::endl;
  else
    context.draw_surface_part(action->surfaces[(int)frame], source, size,
            pos - Vector(action->x_offset, action->y_offset),
            layer + action->z_order, drawing_effect);
}

int
Sprite::get_width() const
{
  return action->surfaces[get_frame()]->w;
}

int
Sprite::get_height() const
{
  return action->surfaces[get_frame()]->h;
}

}

/* EOF */
