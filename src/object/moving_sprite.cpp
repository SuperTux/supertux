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

#include <simplesquirrel/class.hpp>
#include <simplesquirrel/vm.hpp>

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
  m_sprite(),
  m_light_sprites(),
  m_custom_sprites(),
  m_layer(layer_),
  m_flip(NO_FLIP),
  m_sprite_found(false),
  m_custom_layer(false)
{
  change_sprite(m_sprite_name);

  m_col.m_bbox.set_pos(pos);
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
  m_light_sprites(),
  m_custom_sprites(),
  m_layer(layer_),
  m_flip(NO_FLIP),
  m_sprite_found(false),
  m_custom_layer(reader.get("z-pos", m_layer))
{
  m_sprite_found = reader.get("sprite", m_sprite_name);

  //Make the sprite go default when the sprite file is invalid or sprite change fails
  if (m_sprite_name.empty() || !change_sprite(m_sprite_name))
  {
    change_sprite(m_default_sprite_name);
    m_sprite_found = false;
  }

  set_group(collision_group);
}

MovingSprite::MovingSprite(const ReaderMapping& reader, int layer_, CollisionGroup collision_group) :
  MovingObject(reader),
  m_sprite_name(),
  m_default_sprite_name(),
  m_sprite(),
  m_light_sprites(),
  m_custom_sprites(),
  m_layer(layer_),
  m_flip(NO_FLIP),
  m_sprite_found(false),
  m_custom_layer(reader.get("z-pos", m_layer))
{
  m_sprite_found = reader.get("sprite", m_sprite_name);

  if (m_sprite_name.empty() || !change_sprite(m_sprite_name))
    m_sprite_found = false;

  set_group(collision_group);
}

void
MovingSprite::draw(DrawingContext& context)
{
  m_sprite->draw(context.color(), get_pos(), m_layer, m_flip);

  for (auto& sprite : m_custom_sprites)
    sprite->draw(context.color(), get_pos(), m_layer, m_flip);

  for (auto& sprite : m_light_sprites)
    sprite->draw(context.light(), m_col.m_bbox.get_middle(), 0);
}

void
MovingSprite::update(float )
{
}

void
MovingSprite::on_type_change(int old_type)
{
  /** Don't change the sprite/layer to the default one for the current type,
      if this is the initial `on_type_change()` call, and a custom sprite/layer has just been loaded. */
  if (old_type >= 0 || !m_sprite_found)
    change_sprite(get_default_sprite_name());
  if (old_type >= 0 || !m_custom_layer)
    m_layer = get_layer();
}

bool
MovingSprite::matches_sprite(const std::string& sprite_file) const
{
  return m_sprite_name == sprite_file || m_sprite_name == "/" + sprite_file;
}

std::string
MovingSprite::get_action() const
{
  return m_sprite->get_action();
}

void
MovingSprite::update_hitbox()
{
  m_col.set_size(m_sprite->get_current_hitbox_width(), m_sprite->get_current_hitbox_height());
  m_col.set_unisolid(m_sprite->is_current_hitbox_unisolid());
}

void
MovingSprite::set_action(const std::string& name)
{
  if (m_sprite->set_action(name))
    on_sprite_update();
}

void
MovingSprite::set_action(const std::string& name, int loops)
{
  if (m_sprite->set_action(name, loops))
    on_sprite_update();
}

void
MovingSprite::set_action(const std::string& name, const Direction& dir, int loops)
{
  if (m_sprite->set_action(name, dir, loops))
    on_sprite_update();
}

void
MovingSprite::set_action(const Direction& dir, const std::string& name, int loops)
{
  if (m_sprite->set_action(dir, name, loops))
    on_sprite_update();
}

void
MovingSprite::set_action(const Direction& dir, int loops)
{
  if (m_sprite->set_action(dir, loops))
    on_sprite_update();
}

void
MovingSprite::set_action_centered(const std::string& action, int loops)
{
  Vector old_size = m_col.m_bbox.get_size().as_vector();
  if (!m_sprite->set_action(action, loops))
    return;

  on_sprite_update();
  set_pos(get_pos() - (m_col.m_bbox.get_size().as_vector() - old_size) / 2.0f);
}

void
MovingSprite::set_action(const std::string& action, int loops, AnchorPoint anchorPoint)
{
  Rectf old_bbox = m_col.m_bbox;
  if (!m_sprite->set_action(action, loops))
    return;

  on_sprite_update();
  set_pos(get_anchor_pos(old_bbox, m_sprite->get_current_hitbox_width(),
                         m_sprite->get_current_hitbox_height(), anchorPoint));
}

void
MovingSprite::on_sprite_update()
{
  // Update hitbox
  update_hitbox();

  // Update custom sprites
  m_light_sprites = m_sprite->create_custom_linked_sprites(true);
  m_custom_sprites = m_sprite->create_custom_linked_sprites(false);

  // Update other linked sprites
  auto linked_sprites = get_linked_sprites();
  for (const auto& [key, sprite] : linked_sprites)
    sprite = m_sprite->create_linked_sprite(key);
}

bool
MovingSprite::change_sprite(const std::string& new_sprite_name)
{
  m_sprite = SpriteManager::current()->create(new_sprite_name);
  m_sprite_name = new_sprite_name;
  on_sprite_update();

  return m_sprite->load_successful();
}

ObjectSettings
MovingSprite::get_settings()
{
  ObjectSettings result = MovingObject::get_settings();

  result.add_sprite(_("Sprite"), &m_sprite_name, "sprite", get_default_sprite_name());
  result.add_int(_("Z-pos"), &m_layer, "z-pos");

  result.reorder({"sprite", "z-pos", "x", "y"});

  return result;
}

void
MovingSprite::after_editor_set()
{
  MovingObject::after_editor_set();

  std::string current_action = m_sprite->get_action();
  if (!change_sprite(m_sprite_name)) // If sprite change fails, change back to default.
  {
    change_sprite(get_default_sprite_name());
  }
  m_sprite->set_action(current_action);
  on_sprite_update();
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


void
MovingSprite::register_class(ssq::VM& vm)
{
  ssq::Class cls = vm.addAbstractClass<MovingSprite>("MovingSprite", vm.findClass("MovingObject"));

  cls.addFunc("set_sprite", &MovingSprite::change_sprite);
  cls.addFunc("get_sprite", &MovingSprite::get_sprite_name);
  cls.addFunc("get_action", &MovingSprite::get_action);
  cls.addFunc<void, MovingSprite, const std::string&>("set_action", &MovingSprite::set_action);
  cls.addFunc("set_action_loops", &MovingSprite::set_action_loops);

  cls.addVar("sprite", &MovingSprite::get_sprite_name, &MovingSprite::set_sprite);
}

/* EOF */
