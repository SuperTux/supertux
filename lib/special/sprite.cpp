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
#include <cassert>
#include <stdexcept>

#include "app/globals.h"
#include "app/setup.h"
#include "sprite.h"
#include "video/drawing_context.h"

namespace SuperTux
{

Sprite::Sprite(SpriteData& newdata)
  : data(newdata), frame(0), animation_loops(-1)
{
  action = data.get_action("normal");
  if(!action)
    action = data.actions.begin()->second;
  last_ticks = SDL_GetTicks();
}

Sprite::Sprite(const Sprite& other)
  : data(other.data), frame(other.frame),
    animation_loops(other.animation_loops),
    action(other.action)
{
  last_ticks = SDL_GetTicks();
}

Sprite::~Sprite()
{
}

void
Sprite::set_action(std::string name, int loops)
{
  if(action && action->name == name)
    return;

  SpriteData::Action* newaction = data.get_action(name);
  if(!newaction) {
#ifdef DEBUG
    std::cerr << "Action '" << name << "' not found.\n";
#endif
    return;
  }

  action = newaction;
  animation_loops = loops;
  frame = 0;
}

bool
Sprite::check_animation()
{
  return animation_loops == 0;
}

void
Sprite::update()
{
  if(animation_loops == 0)
    return;

  Uint32 ticks = SDL_GetTicks();
  float frame_inc = action->fps * float(ticks - last_ticks)/1000.0;
  last_ticks = ticks;

  frame += frame_inc;

  if(frame >= get_frames()) {
    frame = fmodf(frame+get_frames(), get_frames());
    
    animation_loops--;
    if(animation_loops == 0)
      frame = 0;
  }
}

void
Sprite::draw(DrawingContext& context, const Vector& pos, int layer,
    Uint32 drawing_effect)
{
  assert(action != 0);
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
  assert(action != 0);
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

