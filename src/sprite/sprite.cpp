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

#include "supertux/globals.hpp"
#include "util/log.hpp"
#include "video/surface.hpp"

Sprite::Sprite(SpriteData& newdata) :
  m_data(newdata),
  m_frame(0),
  m_frameidx(0),
  m_animation_loops(-1),
  m_last_ticks(),
  m_angle(0.0f),
  m_alpha(1.0f),
  m_color(1.0f, 1.0f, 1.0f, 1.0f),
  m_blend(),
  m_action(m_data.get_action("normal"))
{
  if (!m_action)
    m_action = m_data.actions.begin()->second.get();
  m_last_ticks = g_game_time;
}

Sprite::Sprite(const Sprite& other) :
  m_data(other.m_data),
  m_frame(other.m_frame),
  m_frameidx(other.m_frameidx),
  m_animation_loops(other.m_animation_loops),
  m_last_ticks(g_game_time),
  m_angle(0.0f), // FIXME: this can't be right
  m_alpha(1.0f),
  m_color(1.0f, 1.0f, 1.0f, 1.0f),
  m_blend(),
  m_action(other.m_action)
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
  if (m_action && m_action->name == name)
    return;

  const SpriteData::Action* newaction = m_data.get_action(name);
  if (!newaction) {
    log_debug << "Action '" << name << "' not found." << std::endl;
    return;
  }

  // If the new action has a loops property,
  // we prefer that over the parameter.
  m_animation_loops = newaction->has_custom_loops ? newaction->loops : loops;

  if (!m_action || m_action->family_name != newaction->family_name)
  {
    m_frame = 0;
    m_frameidx = 0;
  }

  m_action = newaction;
}

void
Sprite::set_action_continued(const std::string& name)
{
  if (m_action && m_action->name == name)
    return;

  const SpriteData::Action* newaction = m_data.get_action(name);
  if (!newaction) {
    log_debug << "Action '" << name << "' not found." << std::endl;
    return;
  }

  m_action = newaction;
  update();
}

bool
Sprite::animation_done() const
{
  return m_animation_loops == 0;
}

void
Sprite::update()
{
  float frame_inc = m_action->fps * (g_game_time - m_last_ticks);
  m_last_ticks = g_game_time;

  m_frame += frame_inc;

  while (m_frame >= 1.0f) {
    m_frame -= 1.0f;
    m_frameidx++;
  }

  while (m_frameidx >= get_frames() && !animation_done()) {
    m_frameidx -= get_frames();
    m_animation_loops--;
  }

  if (animation_done()) {
    m_frame = 0;
    m_frameidx = get_frames() - 1;
  }

  assert(m_frameidx < get_frames());
}

void
Sprite::draw(Canvas& canvas, const Vector& pos, int layer,
             Flip flip)
{
  assert(m_action != nullptr);
  update();


  DrawingContext& context = canvas.get_context();
  context.push_transform();

  context.set_flip(context.get_flip() ^ flip);
  context.set_alpha(context.get_alpha() * m_alpha);

  canvas.draw_surface(m_action->surfaces[m_frameidx],
                      pos - Vector(m_action->x_offset, m_action->y_offset),
                      m_angle,
                      m_color,
                      m_blend,
                      layer);

  context.pop_transform();
}

int
Sprite::get_width() const
{
  assert(m_frameidx < get_frames());
  return static_cast<int>(m_action->surfaces[m_frameidx]->get_width());
}

int
Sprite::get_height() const
{
  assert(m_frameidx < get_frames());
  return static_cast<int>(m_action->surfaces[m_frameidx]->get_height());
}

float
Sprite::get_current_hitbox_x_offset() const
{
  return m_action->x_offset;
}

float
Sprite::get_current_hitbox_y_offset() const
{
  return m_action->y_offset;
}

float
Sprite::get_current_hitbox_width() const
{
  return m_action->hitbox_w;
}

float
Sprite::get_current_hitbox_height() const
{
  return m_action->hitbox_h;
}

Rectf
Sprite::get_current_hitbox() const
{
  return Rectf(m_action->x_offset, m_action->y_offset, m_action->x_offset + m_action->hitbox_w, m_action->y_offset + m_action->hitbox_h);
}

void
Sprite::set_angle(float a)
{
  m_angle = a;
}

float
Sprite::get_angle() const
{
  return m_angle;
}

void
Sprite::set_alpha(float a)
{
  m_alpha = a;
}

float
Sprite::get_alpha() const
{
  return m_alpha;
}

void
Sprite::set_color(const Color& c)
{
  m_color = c;
}

Color
Sprite::get_color() const
{
  return m_color;
}

void
Sprite::set_blend(const Blend& b)
{
  m_blend = b;
}

Blend
Sprite::get_blend() const
{
  return m_blend;
}

/* EOF */
