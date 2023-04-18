//  SuperTux - MovingSprite Base Class
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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

#include "object/moving_sprite.hpp"

#include <math.h>
#include <physfs.h>

#include "editor/editor.hpp"
#include "math/random.hpp"
#include "math/util.hpp"
#include "object/sprite_particle.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"
#include "util/writer.hpp"

MovingSprite::MovingSprite(const Vector& pos, const std::string& sprite_name_,
                           int layer_, CollisionGroup collision_group) :
  m_sprite_name(sprite_name_),
  m_default_sprite_name(sprite_name_),
  m_sprite(SpriteManager::current()->create(m_sprite_name)),
  m_layer(layer_),
  m_flip(NO_FLIP),
  m_sprite_found(false)
{
  m_col.m_bbox.set_pos(pos);
  m_col.m_bbox.set_size(m_sprite->get_current_hitbox_width(), m_sprite->get_current_hitbox_height());
  set_group(collision_group);
}

MovingSprite::MovingSprite(const ReaderMapping& reader, const Vector& pos, int layer_, CollisionGroup collision_group) :
  MovingSprite(reader, layer_, collision_group)
{
  m_col.m_bbox.set_pos(pos);
}

MovingSprite::MovingSprite(const ReaderMapping& reader, const std::string& sprite_name_, int layer_, CollisionGroup collision_group) :
  MovingObject(reader),
  m_sprite_name(sprite_name_),
  m_default_sprite_name(sprite_name_),
  m_sprite(),
  m_layer(layer_),
  m_flip(NO_FLIP),
  m_sprite_found(false)
{
  reader.get("x", m_col.m_bbox.get_left());
  reader.get("y", m_col.m_bbox.get_top());
  m_sprite_found = reader.get("sprite", m_sprite_name);

  //Make the sprite go default when the sprite file is invalid
  if (m_sprite_name.empty() || !PHYSFS_exists(m_sprite_name.c_str()))
  {
    m_sprite = SpriteManager::current()->create(m_default_sprite_name);
    m_sprite_found = false;
  }
  else
  {
    if (!change_sprite(m_sprite_name)) // If sprite change fails, change back to default.
    {
      m_sprite = SpriteManager::current()->create(m_default_sprite_name);
      m_sprite_found = false;
    }
  }

  m_col.m_bbox.set_size(m_sprite->get_current_hitbox_width(), m_sprite->get_current_hitbox_height());
  set_group(collision_group);
}

MovingSprite::MovingSprite(const ReaderMapping& reader, int layer_, CollisionGroup collision_group) :
  MovingObject(reader),
  m_sprite_name(),
  m_default_sprite_name(),
  m_sprite(),
  m_layer(layer_),
  m_flip(NO_FLIP),
  m_sprite_found(false)
{
  reader.get("x", m_col.m_bbox.get_left());
  reader.get("y", m_col.m_bbox.get_top());
  if (!reader.get("sprite", m_sprite_name))
    throw std::runtime_error("no sprite name set");

  m_sprite_found = true;
  //m_default_sprite_name = m_sprite_name;
  m_sprite = SpriteManager::current()->create(m_sprite_name);
  m_col.m_bbox.set_size(m_sprite->get_current_hitbox_width(), m_sprite->get_current_hitbox_height());
  set_group(collision_group);
}

void
MovingSprite::draw(DrawingContext& context)
{
  m_sprite->draw(context.color(), get_pos(), m_layer, m_flip);
}

void
MovingSprite::update(float )
{
}

bool
MovingSprite::has_found_sprite()
{
  bool found = m_sprite_found;
  m_sprite_found = false; // After the first call, indicate that a custom sprite has not been found.
  return found;
}

std::string
MovingSprite::get_sprite_name() const
{
  return m_sprite_name;
}

void
MovingSprite::set_action(const std::string& action, int loops)
{
  m_sprite->set_action(action, loops);
  m_col.set_size(m_sprite->get_current_hitbox_width(), m_sprite->get_current_hitbox_height());
}

void
MovingSprite::set_action_centered(const std::string& action, int loops)
{
  Vector old_size = m_col.m_bbox.get_size().as_vector();
  m_sprite->set_action(action, loops);
  m_col.set_size(m_sprite->get_current_hitbox_width(), m_sprite->get_current_hitbox_height());
  set_pos(get_pos() - (m_col.m_bbox.get_size().as_vector() - old_size) / 2.0f);
}

void
MovingSprite::set_action(const std::string& action, int loops, AnchorPoint anchorPoint)
{
  Rectf old_bbox = m_col.m_bbox;
  m_sprite->set_action(action, loops);
  float w = m_sprite->get_current_hitbox_width();
  float h = m_sprite->get_current_hitbox_height();
  m_col.set_size(w, h);
  set_pos(get_anchor_pos(old_bbox, w, h, anchorPoint));
}

bool
MovingSprite::change_sprite(const std::string& new_sprite_name)
{
  SpritePtr new_sprite;
  try
  {
    new_sprite = SpriteManager::current()->create(m_sprite_name);
  }
  catch (std::exception& err)
  {
    log_warning << "Sprite change failed: Sprite '" << new_sprite_name << "' cannot be loaded: " << err.what() << std::endl;
    return false;
  }

  m_sprite = std::move(new_sprite);
  m_sprite_name = new_sprite_name;
  return true;
}

ObjectSettings
MovingSprite::get_settings()
{
  ObjectSettings result = MovingObject::get_settings();

  result.add_sprite(_("Sprite"), &m_sprite_name, "sprite", m_default_sprite_name);

  result.reorder({"sprite", "x", "y"});

  return result;
}

void
MovingSprite::after_editor_set()
{
  MovingObject::after_editor_set();

  std::string current_action = m_sprite->get_action();
  m_sprite = SpriteManager::current()->create(m_sprite_name);
  m_sprite->set_action(current_action);

  m_col.m_bbox.set_size(m_sprite->get_current_hitbox_width(), m_sprite->get_current_hitbox_height());
}

void
MovingSprite::spawn_explosion_sprites(int count, const std::string& sprite_path)
{
    for (int i = 0; i < count; i++) {
      Vector ppos = m_col.m_bbox.get_middle();
      float angle = graphicsRandom.randf(-math::PI_2, math::PI_2);
      float velocity = graphicsRandom.randf(350, 400);
      float vx = sinf(angle)*velocity;
      float vy = -cosf(angle)*velocity;
      Vector pspeed = Vector(vx, vy);
      Vector paccel = Vector(0, Sector::get().get_gravity()*10);
      Sector::get().add<SpriteParticle>(sprite_path,
                                             "default",
                                             ppos, ANCHOR_MIDDLE,
                                             pspeed, paccel,
                                             LAYER_OBJECTS-1);
  }
}

/* EOF */
