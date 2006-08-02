//  $Id$
//
//  SuperTux - MovingSprite Base Class
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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

#include <stdexcept>

#include "moving_sprite.hpp"
#include "video/drawing_context.hpp"
#include "sprite/sprite_manager.hpp"
#include "player.hpp"
#include "sector.hpp"
#include "player_status.hpp"
#include "gameobjs.hpp"
#include "statistics.hpp"
#include "object_factory.hpp"
#include "level.hpp"
#include "random_generator.hpp"
#include "audio/sound_source.hpp"
#include "audio/sound_manager.hpp"
#include "timer.hpp"

MovingSprite::MovingSprite(const Vector& pos, const std::string& sprite_name, int layer, CollisionGroup collision_group)
	: sprite_name(sprite_name), layer(layer)
{
  bbox.set_pos(pos);
  sprite = sprite_manager->create(sprite_name);
  bbox.set_size(sprite->get_current_hitbox_width(), sprite->get_current_hitbox_height());
  set_group(collision_group);
}

MovingSprite::MovingSprite(const lisp::Lisp& reader, const Vector& pos, int layer, CollisionGroup collision_group)
	: layer(layer)
{
  bbox.set_pos(pos);
  if (!reader.get("sprite", sprite_name))
    throw std::runtime_error("no sprite name set");

  sprite = sprite_manager->create(sprite_name);
  bbox.set_size(sprite->get_current_hitbox_width(), sprite->get_current_hitbox_height());
  set_group(collision_group);
}

MovingSprite::MovingSprite(const lisp::Lisp& reader, const std::string& sprite_name, int layer, CollisionGroup collision_group)
	: sprite_name(sprite_name), layer(layer)
{
  if (!reader.get("x", bbox.p1.x))
    throw std::runtime_error("no x position set");
  if (!reader.get("y", bbox.p1.y))
    throw std::runtime_error("no y position set");

  sprite = sprite_manager->create(sprite_name);
  bbox.set_size(sprite->get_current_hitbox_width(), sprite->get_current_hitbox_height());
  set_group(collision_group);
}

MovingSprite::MovingSprite(const lisp::Lisp& reader, int layer, CollisionGroup collision_group)
	: layer(layer)
{
  if (!reader.get("x", bbox.p1.x))
    throw std::runtime_error("no x position set");
  if (!reader.get("y", bbox.p1.y))
    throw std::runtime_error("no y position set");
  if (!reader.get("sprite", sprite_name))
    throw std::runtime_error("no sprite name set");

  sprite = sprite_manager->create(sprite_name);
  bbox.set_size(sprite->get_current_hitbox_width(), sprite->get_current_hitbox_height());
  set_group(collision_group);
}

MovingSprite::MovingSprite(const MovingSprite& other)
	: MovingObject(other), layer(other.layer)
{
  sprite = new Sprite(*other.sprite);
}

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

MovingSprite::~MovingSprite()
{
  delete sprite;
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

void
MovingSprite::set_action(const std::string& action, int loops)
{
  sprite->set_action(action, loops);
  set_size(sprite->get_current_hitbox_width(), sprite->get_current_hitbox_height());
}

void
MovingSprite::set_action_centered(const std::string& action, int loops)
{
  Vector old_size = bbox.get_size();
  sprite->set_action(action, loops);
  set_size(sprite->get_current_hitbox_width(), sprite->get_current_hitbox_height());
  set_pos(get_pos() - (bbox.get_size() - old_size) / 2);
}
