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

#include "sprite/sprite_manager.hpp"
#include "supertux/direction.hpp"
#include "supertux/globals.hpp"
#include "util/log.hpp"
#include "video/surface.hpp"

Sprite::Sprite(SpriteData& newdata) :
  m_data(newdata),
  m_frame(0),
  m_frameidx(0),
  m_last_ticks(),
  m_is_paused(false),
  m_animation_loops(-1),
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
  m_last_ticks(g_game_time),
  m_animation_loops(other.m_animation_loops),
  m_angle(0.0f), // FIXME: this can't be right
  m_alpha(1.0f),
  m_color(1.0f, 1.0f, 1.0f, 1.0f),
  m_blend(),
  m_is_paused(other.m_is_paused),
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
Sprite::apply_config(const SpriteConfig& config)
{
  set_action(config.action);

  m_animation_loops = config.loops;
  m_angle = config.angle;
  m_alpha = config.alpha;
  m_color = config.color;
  m_blend = config.blend;
}

bool
Sprite::set_action(const std::string& name, const Direction& dir, int loops)
{
  if (dir == Direction::NONE)
    return set_action(name, loops);
  else
    return set_action(name + "-" + dir_to_string(dir), loops);
}

bool
Sprite::set_action(const Direction& dir, const std::string& name, int loops)
{
  if (dir == Direction::NONE)
    return set_action(name, loops);
  else
    return set_action(dir_to_string(dir) + "-" + name, loops);
}

bool
Sprite::set_action(const Direction& dir, int loops)
{
  return set_action(dir_to_string(dir), loops);
}

bool
Sprite::set_action(const std::string& name, int loops)
{
  if (m_action && m_action->name == name)
    return false;

  const SpriteData::Action* newaction = m_data.get_action(name);
  if (!newaction) {
    // HACK: Lots of things trigger this message therefore turning it into a warning
    // would make it quite annoying
    log_debug << m_data.m_filename << ": Action '" << name << "' not found." << std::endl;
    return false;
  }

  // Automatically resume if a new action is set
  m_is_paused = false;

  // The action's loops were set to continued; use the ones from the previous action.
  if (loops == LOOPS_CONTINUED)
  {
    m_action = newaction;
    update();
    return true;
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
  return true;
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

  if (m_is_paused)
    return;

  m_frame += frame_inc;

  while (m_frame >= 1.0f) {
    m_frame -= 1.0f;
    m_frameidx++;
  }

  while (m_frameidx >= get_frames() && !animation_done()) {
    // Loop animation.
    m_frameidx -= get_frames() - (m_action->loop_frame - 1);
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
  assert(m_action);
  update();

  DrawingContext& context = canvas.get_context();
  context.push_transform();

  context.set_flip(context.get_flip() ^ flip);
  context.set_alpha(context.get_alpha() * m_alpha);

  canvas.draw_surface(m_action->surfaces[m_frameidx],
                      pos - Vector(m_action->x_offset, flip == NO_FLIP ? m_action->y_offset :
                                   (static_cast<float>(m_action->surfaces[m_frameidx]->get_height()) - m_action->y_offset - m_action->hitbox_h + m_action->flip_offset)),
                      m_angle,
                      m_color,
                      m_blend,
                      layer);

  context.pop_transform();
}

void
Sprite::draw_scaled(Canvas& canvas, const Rectf& dest_rect, int layer,
                    Flip flip)
{
  assert(m_action);
  update();

  DrawingContext& context = canvas.get_context();
  context.push_transform();

  context.set_flip(context.get_flip() ^ flip);
  context.set_alpha(context.get_alpha() * m_alpha);

  PaintStyle style;
  style.set_color(m_color);
  style.set_alpha(m_color.alpha);
  style.set_blend(m_blend);

  canvas.draw_surface_scaled(m_action->surfaces[m_frameidx], dest_rect, layer, style);

  context.pop_transform();
}

const std::vector<SpriteData::LinkedSprite>&
Sprite::get_custom_linked_sprites() const
{
  return m_action->custom_linked_sprites.empty() ? m_data.custom_linked_sprites : m_action->custom_linked_sprites;
}

std::vector<SpritePtr>
Sprite::create_custom_linked_sprites(bool light) const
{
  const auto& sprites_data = get_custom_linked_sprites();

  std::vector<SpritePtr> result;
  for (const SpriteData::LinkedSprite& sprite_data : sprites_data)
  {
    if (sprite_data.light != light)
      continue;

    SpritePtr sprite = SpriteManager::current()->create(sprite_data.file);
    sprite->apply_config(sprite_data.config);
    if (sprite_data.light)
      sprite->set_blend(Blend::ADD);

    result.push_back(std::move(sprite));
  }
  return result;
}

bool
Sprite::has_linked_sprite(const std::string& key) const
{
  return m_action->linked_sprites.find(key) != m_action->linked_sprites.end() ||
         m_data.linked_sprites.find(key) != m_data.linked_sprites.end();
}

const SpriteData::LinkedSprite&
Sprite::get_linked_sprite(const std::string& key) const
{
  auto it = m_action->linked_sprites.find(key);
  if (it != m_action->linked_sprites.end())
    return it->second;

  it = m_data.linked_sprites.find(key);
  if (it == m_data.linked_sprites.end()) // No linked sprite with such key
  {
    log_warning << m_data.m_filename << ": No linked sprite with key '" << key << "'." << std::endl;

    static SpriteData::LinkedSprite dummy_sprite_data;
    return dummy_sprite_data; // Empty linked sprite with an empty filename leads to a dummy sprite
  }

  return it->second;
}

SpritePtr
Sprite::create_linked_sprite(const std::string& key) const
{
  const auto& sprite_data = get_linked_sprite(key);

  SpritePtr sprite = SpriteManager::current()->create(sprite_data.file);
  sprite->apply_config(sprite_data.config);

  return sprite;
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

const std::optional<std::vector<SurfacePtr>>
Sprite::get_action_surfaces(const std::string& name) const
{
  const SpriteData::Action* action = m_data.get_action(name);
  if (!action) return std::nullopt;
  return action->surfaces;
}

Rectf
Sprite::get_current_hitbox() const
{
  return Rectf(m_action->x_offset, m_action->y_offset, m_action->x_offset + m_action->hitbox_w, m_action->y_offset + m_action->hitbox_h);
}

/* EOF */
