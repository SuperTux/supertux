//  SuperTux
//  Copyright (C) 2022 A. Semphris <semphris@protonmail.com>
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

#include "object/lit_object.hpp"

#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/flip_level_transformer.hpp"
#include "util/reader_mapping.hpp"

LitObject::LitObject(const ReaderMapping& reader) :
  MovingObject(reader),
  ExposedObject<LitObject, scripting::LitObject>(this),
  m_light_offset(-6.f, -17.f),
  m_sprite_name("images/objects/lightflower/lightflower1.sprite"),
  m_light_sprite_name("images/objects/lightflower/light/glow_light.sprite"),
  m_sprite_action("default"),
  m_light_sprite_action("default"),
  m_sprite(),
  m_light_sprite(),
  m_layer(0),
  m_flip(NO_FLIP)
{
  reader.get("x", m_col.m_bbox.get_left());
  reader.get("y", m_col.m_bbox.get_top());

  reader.get("light-offset-x", m_light_offset.x);
  reader.get("light-offset-y", m_light_offset.y);

  reader.get("sprite", m_sprite_name);
  reader.get("light-sprite", m_light_sprite_name);
  reader.get("layer", m_layer, 0);

  reader.get("action", m_sprite_action);
  reader.get("light-action", m_light_sprite_action);

  m_sprite = SpriteManager::current()->create(m_sprite_name);
  m_light_sprite = SpriteManager::current()->create(m_light_sprite_name);
  m_light_sprite->set_blend(Blend::ADD);

  m_sprite->set_action(m_sprite_action);
  m_light_sprite->set_action(m_light_sprite_action);

  m_col.m_bbox.set_size(static_cast<float>(m_sprite->get_width()),
                        static_cast<float>(m_sprite->get_height()));

  set_group(COLGROUP_DISABLED);
}

void
LitObject::draw(DrawingContext& context)
{
  m_sprite->draw(context.color(), get_pos(), m_layer - 1, m_flip);
  m_light_sprite->draw(context.light(), get_pos() - m_light_offset, m_layer - 1, m_flip);
}

void
LitObject::update(float)
{
}

ObjectSettings
LitObject::get_settings()
{
  ObjectSettings result = MovingObject::get_settings();

  result.add_sprite(_("Sprite"), &m_sprite_name, "sprite", std::string("images/objects/lightflower/lightflower1.sprite"));
  result.add_sprite(_("Light sprite"), &m_light_sprite_name, "light-sprite", std::string("images/objects/lightflower/light/glow_light.sprite"));
  result.add_int(_("Layer"), &m_layer, "layer", 0);

  result.add_text(_("Sprite starting action"), &m_sprite_action, "action", std::string("default"));
  result.add_text(_("Light sprite starting action"), &m_light_sprite_action, "light-action", std::string("default"));

  result.add_float(_("Light sprite offset X"), &m_light_offset.x, "light-offset-x");
  result.add_float(_("Light sprite offset Y"), &m_light_offset.y, "light-offset-y");

  return result;
}

void
LitObject::after_editor_set()
{
  m_sprite = SpriteManager::current()->create(m_sprite_name);
  m_light_sprite = SpriteManager::current()->create(m_light_sprite_name);
  m_light_sprite->set_blend(Blend::ADD);

  m_sprite->set_action(m_sprite_action);
  m_light_sprite->set_action(m_light_sprite_action);

  m_col.m_bbox.set_size(static_cast<float>(m_sprite->get_width()),
                        static_cast<float>(m_sprite->get_height()));
}

void
LitObject::on_flip(float height)
{
  MovingObject::on_flip(height);
  FlipLevelTransformer::transform_flip(m_flip);
}

const std::string&
LitObject::get_action() const
{
  return m_sprite->get_action();
}

void
LitObject::set_action(const std::string& action)
{
  m_sprite->set_action(action);
}

const std::string&
LitObject::get_light_action() const
{
  return m_light_sprite->get_action();
}

void
LitObject::set_light_action(const std::string& action)
{
  m_light_sprite->set_action(action);
}

/* EOF */
