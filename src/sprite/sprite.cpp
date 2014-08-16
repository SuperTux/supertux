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

#include "sprite/sprite.hpp"

#include <assert.h>
#include <math.h>

#include "supertux/timer.hpp"

Sprite::Sprite(SpriteData& newdata) :
  data(newdata),
  frame(0),
  frameidx(0),
  animation_loops(-1),
  last_ticks(),
  angle(0.0f),
  color(1.0f, 1.0f, 1.0f, 1.0f),
  blend(),
  action(data.get_action("normal"))
{
  if(!action)
    action = data.actions.begin()->second;
  last_ticks = game_time;
}

Sprite::Sprite(const Sprite& other) :
  data(other.data),
  frame(other.frame),
  frameidx(other.frameidx),
  animation_loops(other.animation_loops),
  last_ticks(game_time),
  angle(0.0f), // FIXME: this can't be right
  color(1.0f, 1.0f, 1.0f, 1.0f),
  blend(),
  action(other.action)
{
}

Sprite::~Sprite()
{
}

SpritePtr
Sprite::clone() const
{
  return SpritePtr(new Sprite(*this));
}

void
Sprite::set_action(const std::string& name, int loops)
{
  if(action && action->name == name)
    return;

  const SpriteData::Action* newaction = data.get_action(name);
  if(!newaction) {
    log_debug << "Action '" << name << "' not found." << std::endl;
    return;
  }

  action = newaction;
  animation_loops = loops;
  frame = 0;
  frameidx = 0;
}

void
Sprite::set_action_continued(const std::string& name)
{
  if(action && action->name == name)
    return;

  const SpriteData::Action* newaction = data.get_action(name);
  if(!newaction) {
    log_debug << "Action '" << name << "' not found." << std::endl;
    return;
  }

  action = newaction;
  update();
}

bool
Sprite::animation_done()
{
  return animation_loops == 0;
}

void
Sprite::update()
{
  float frame_inc = action->fps * (game_time - last_ticks);
  last_ticks = game_time;

  frame += frame_inc;

  while(frame >= 1.0f) {
    frame -= 1.0f;
    frameidx++;
  }

  while(frameidx >= get_frames()) {
    frameidx -= get_frames();
    animation_loops--;
    if(animation_done()) {
      break;
    }
  }

  if(animation_done()) {
    frame = 0;
    frameidx = get_frames()-1;
  }

  assert(frameidx < get_frames());
}

void
Sprite::draw(DrawingContext& context, const Vector& pos, int layer,
             DrawingEffect effect)
{
  assert(action != 0);
  update();

  context.set_drawing_effect(effect);
  context.draw_surface(action->surfaces[frameidx],
                       pos - Vector(action->x_offset, action->y_offset),
                       angle,
                       color,
                       blend,
                       layer + action->z_order);
}

void
Sprite::draw_part(DrawingContext& context, const Vector& source,
                  const Vector& size, const Vector& pos, int layer)
{
  assert(action != 0);
  update();

  context.draw_surface_part(action->surfaces[frameidx],
                            Rectf(source, Sizef(size)),
                            Rectf(pos - Vector(action->x_offset, action->y_offset),
                                  action->surfaces[frameidx]->get_size()),
                            layer + action->z_order);
}

int
Sprite::get_width() const
{
  assert(frameidx < get_frames());
  return (int) action->surfaces[get_frame()]->get_width();
}

int
Sprite::get_height() const
{
  assert(frameidx < get_frames());
  return (int) action->surfaces[get_frame()]->get_height();
}

float
Sprite::get_current_hitbox_x_offset() const
{
  return action->x_offset;
}

float
Sprite::get_current_hitbox_y_offset() const
{
  return action->y_offset;
}

float
Sprite::get_current_hitbox_width() const
{
  return action->hitbox_w;
}

float
Sprite::get_current_hitbox_height() const
{
  return action->hitbox_h;
}

Rectf
Sprite::get_current_hitbox() const
{
  return Rectf(action->x_offset, action->y_offset, action->x_offset + action->hitbox_w, action->y_offset + action->hitbox_h);
}

void
Sprite::set_angle(float a)
{
  angle = a;
}

float
Sprite::get_angle() const
{
  return angle;
}

void
Sprite::set_color(const Color& c)
{
  color = c;
}

Color
Sprite::get_color() const
{
  return color;
}

void
Sprite::set_blend(const Blend& b)
{
  blend = b;
}

Blend
Sprite::get_blend() const
{
  return blend;
}

/* EOF */
