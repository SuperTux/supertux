//  SuperTux
//  Copyright (C) 2006 Ingo Ruhnke <grumbel@gmail.com>
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

#include "object/spotlight.hpp"

#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "util/reader_mapping.hpp"

Spotlight::Direction
Spotlight::Direction_from_string(const std::string& s)
{
  if (s == "clockwise") {
    return Direction::CLOCKWISE;
  } else if (s == "counter-clockwise") {
    return Direction::COUNTERCLOCKWISE;
  } else if (s == "stopped") {
    return Direction::STOPPED;
  }

  throw std::runtime_error("Invalid spotlight direction from string '" + s + "'");
}

std::string
Spotlight::Direction_to_string(Direction dir)
{
  switch(dir) {
    case Direction::CLOCKWISE:
      return "clockwise";
    case Direction::COUNTERCLOCKWISE:
      return "counter-clockwise";
    case Direction::STOPPED:
      return "stopped";
  }

  throw std::runtime_error("Invalid spotlight direction '" + std::to_string(static_cast<int>(dir)) + "'");
}

Spotlight::Spotlight(const ReaderMapping& mapping) :
  MovingObject(mapping),
  ExposedObject<Spotlight, scripting::Spotlight>(this),
  angle(),
  center(SpriteManager::current()->create("images/objects/spotlight/spotlight_center.sprite")),
  base(SpriteManager::current()->create("images/objects/spotlight/spotlight_base.sprite")),
  lights(SpriteManager::current()->create("images/objects/spotlight/spotlight_lights.sprite")),
  light(SpriteManager::current()->create("images/objects/spotlight/light.sprite")),
  lightcone(SpriteManager::current()->create("images/objects/spotlight/lightcone.sprite")),
  color(1.0f, 1.0f, 1.0f),
  speed(50.0f),
  m_direction(),
  m_layer(0),
  m_enabled(true)
{
  m_col.m_group = COLGROUP_DISABLED;

  mapping.get("x", m_col.m_bbox.get_left(), 0.0f);
  mapping.get("y", m_col.m_bbox.get_top(), 0.0f);
  m_col.m_bbox.set_size(32, 32);

  mapping.get("angle", angle, 0.0f);
  mapping.get("speed", speed, 50.0f);

  if (!mapping.get_custom("r-direction", m_direction, Direction_from_string))
  {
    // Retrocompatibility
    bool counter_clockwise;
    mapping.get("counter-clockwise", counter_clockwise, false);
    m_direction = counter_clockwise ? Direction::COUNTERCLOCKWISE : Direction::CLOCKWISE;
  }

  std::vector<float> vColor;
  if ( mapping.get( "color", vColor ) ){
    color = Color( vColor );
  }

  mapping.get("layer", m_layer, 0);
  mapping.get("enabled", m_enabled, true);
}

Spotlight::~Spotlight()
{
}

ObjectSettings
Spotlight::get_settings()
{
  ObjectSettings result = MovingObject::get_settings();

  result.add_bool(_("Enabled"), &m_enabled, "enabled", true);
  result.add_float(_("Angle"), &angle, "angle");
  result.add_color(_("Color"), &color, "color", Color::WHITE);
  result.add_float(_("Speed"), &speed, "speed", 50.0f);
  result.add_enum(_("Direction"), reinterpret_cast<int*>(&m_direction),
                  {_("Clockwise"), _("Counter-clockwise"), _("Stopped")},
                  {"clockwise", "counter-clockwise", "stopped"},
                  static_cast<int>(Direction::CLOCKWISE), "r-direction");
  result.add_int(_("Layer"), &m_layer, "layer", 0);

  result.reorder({"angle", "color", "layer", "x", "y"});

  return result;
}

void
Spotlight::update(float dt_sec)
{
  GameObject::update(dt_sec);

  if (!m_enabled)
    return;

  switch (m_direction)
  {
  case Direction::CLOCKWISE:
    angle += dt_sec * speed;
    break;

  case Direction::COUNTERCLOCKWISE:
    angle -= dt_sec * speed;
    break;
  
  case Direction::STOPPED:
    break;
  }
}

void
Spotlight::draw(DrawingContext& context)
{
  if (m_enabled)
  {
    light->set_color(color);
    light->set_blend(Blend::ADD);
    light->set_angle(angle);
    light->draw(context.light(), m_col.m_bbox.p1(), m_layer);

    //lightcone->set_angle(angle);
    //lightcone->draw(context.color(), position, m_layer);

    lights->set_angle(angle);
    lights->draw(context.color(), m_col.m_bbox.p1(), m_layer);
  }

  base->set_angle(angle);
  base->draw(context.color(), m_col.m_bbox.p1(), m_layer);

  center->draw(context.color(), m_col.m_bbox.p1(), m_layer);

  if (m_enabled)
  {
    lightcone->set_angle(angle);
    lightcone->draw(context.color(), m_col.m_bbox.p1(), LAYER_FOREGROUND1 + 10);
  }
}

HitResponse
Spotlight::collision(GameObject& other, const CollisionHit& hit_)
{
  return FORCE_MOVE;
}

/* EOF */
