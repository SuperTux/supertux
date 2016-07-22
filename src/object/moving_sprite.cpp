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

#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "util/log.hpp"
#include "util/reader_mapping.hpp"

#include <physfs.h>
#include <stdexcept>

MovingSprite::MovingSprite(const Vector& pos, const std::string& sprite_name_,
                           int layer_, CollisionGroup collision_group) :
  sprite_name(sprite_name_),
  sprite(),
  layer(layer_)
{
  bbox.set_pos(pos);
  sprite = SpriteManager::current()->create(sprite_name);
  bbox.set_size(sprite->get_current_hitbox_width(), sprite->get_current_hitbox_height());
  set_group(collision_group);
}

MovingSprite::MovingSprite(const ReaderMapping& reader, const Vector& pos, int layer_, CollisionGroup collision_group) :
  sprite_name(),
  sprite(),
  layer(layer_)
{
  bbox.set_pos(pos);
  if (!reader.get("sprite", sprite_name))
    throw std::runtime_error("no sprite name set");

  sprite = SpriteManager::current()->create(sprite_name);
  bbox.set_size(sprite->get_current_hitbox_width(), sprite->get_current_hitbox_height());
  set_group(collision_group);
}

MovingSprite::MovingSprite(const ReaderMapping& reader, const std::string& sprite_name_, int layer_, CollisionGroup collision_group) :
  sprite_name(sprite_name_),
  sprite(),
  layer(layer_)
{
  reader.get("x", bbox.p1.x);
  reader.get("y", bbox.p1.y);
  reader.get("sprite", this->sprite_name);

  //make the sprite go defaut when the sprite file is invalid
  if (sprite_name.empty() || !PHYSFS_exists(sprite_name.c_str())) {
    sprite_name = sprite_name_;
  }

  sprite = SpriteManager::current()->create(this->sprite_name);
  bbox.set_size(sprite->get_current_hitbox_width(), sprite->get_current_hitbox_height());
  set_group(collision_group);
}

MovingSprite::MovingSprite(const ReaderMapping& reader, int layer_, CollisionGroup collision_group) :
  sprite_name(),
  sprite(),
  layer(layer_)
{
  reader.get("x", bbox.p1.x);
  reader.get("y", bbox.p1.y);
  if (!reader.get("sprite", sprite_name))
    throw std::runtime_error("no sprite name set");

  sprite = SpriteManager::current()->create(sprite_name);
  bbox.set_size(sprite->get_current_hitbox_width(), sprite->get_current_hitbox_height());
  set_group(collision_group);
}

MovingSprite::MovingSprite(const MovingSprite& other) :
  MovingObject(other),
  sprite_name(),
  sprite(other.sprite->clone()),
  layer(other.layer)
{
}
/*
  MovingSprite&
  MovingSprite::operator=(const MovingSprite& other)
  {
  if (this == &other)
  return *this;

  delete sprite;
  sprite = new Sprite(*other.sprite);

  layer = other.layer;

  return *this;
  }
*/
MovingSprite::~MovingSprite()
{
}

void
MovingSprite::draw(DrawingContext& context)
{
  sprite->draw(context, get_pos(), layer);
}

void
MovingSprite::update(float )
{
}

std::string
MovingSprite::get_sprite_name() const
{
  return sprite_name;
}

void
MovingSprite::set_action(const std::string& action, int loops)
{
  sprite->set_action(action, loops);
  set_size(sprite->get_current_hitbox_width(), sprite->get_current_hitbox_height());
}

void
MovingSprite::set_action_centered(const std::string& action, int loops)
{
  Vector old_size = bbox.get_size().as_vector();
  sprite->set_action(action, loops);
  set_size(sprite->get_current_hitbox_width(), sprite->get_current_hitbox_height());
  set_pos(get_pos() - (bbox.get_size().as_vector() - old_size) / 2);
}

void
MovingSprite::set_action(const std::string& action, int loops, AnchorPoint anchorPoint)
{
  Rectf old_bbox = bbox;
  sprite->set_action(action, loops);
  float w = sprite->get_current_hitbox_width();
  float h = sprite->get_current_hitbox_height();
  set_size(w, h);
  set_pos(get_anchor_pos(old_bbox, w, h, anchorPoint));
}

void MovingSprite::change_sprite(const std::string& new_sprite_name)
{
  sprite_name = new_sprite_name;
  sprite = SpriteManager::current()->create(sprite_name);
}

ObjectSettings MovingSprite::get_settings()
{
  ObjectSettings result = MovingObject::get_settings();
  ObjectOption spr(MN_FILE, _("Sprite"), &sprite_name, "sprite");
  spr.select.push_back(".sprite");
  result.options.push_back(spr);
  return result;
}

void MovingSprite::after_editor_set()
{
  std::string current_action = sprite->get_action();
  sprite = SpriteManager::current()->create(sprite_name);
  sprite->set_action(current_action);
}

/* EOF */
