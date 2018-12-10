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

Spotlight::Spotlight(const ReaderMapping& mapping) :
  angle(),
  center(SpriteManager::current()->create("images/objects/spotlight/spotlight_center.sprite")),
  base(SpriteManager::current()->create("images/objects/spotlight/spotlight_base.sprite")),
  lights(SpriteManager::current()->create("images/objects/spotlight/spotlight_lights.sprite")),
  light(SpriteManager::current()->create("images/objects/spotlight/light.sprite")),
  lightcone(SpriteManager::current()->create("images/objects/spotlight/lightcone.sprite")),
  color(1.0f, 1.0f, 1.0f),
  speed(50.0f),
  counter_clockwise()
{
  m_col.m_group = COLGROUP_DISABLED;

  mapping.get("x", m_col.m_bbox.get_left(), 0.0f);
  mapping.get("y", m_col.m_bbox.get_top(), 0.0f);
  m_col.m_bbox.set_size(32, 32);

  mapping.get("angle", angle, 0.0f);
  mapping.get("speed", speed, 50.0f);
  mapping.get("counter-clockwise", counter_clockwise, false);

  std::vector<float> vColor;
  if ( mapping.get( "color", vColor ) ){
    color = Color( vColor );
  }
}

Spotlight::~Spotlight()
{
}

ObjectSettings
Spotlight::get_settings()
{
  ObjectSettings result = MovingObject::get_settings();

  result.add_float(_("Angle"), &angle, "angle");
  result.add_color(_("Color"), &color, "color", Color::WHITE);
  result.add_float(_("Speed"), &speed, "speed", 50.0f);
  result.add_bool(_("Counter-clockwise"), &counter_clockwise, "counter-clockwise", false);

  result.reorder({"angle", "color", "x", "y"});

  return result;
}

void
Spotlight::update(float dt_sec)
{
  if (counter_clockwise)
  {
    angle -= dt_sec * speed;
  }
  else
  {
    angle += dt_sec * speed;
  }
}

void
Spotlight::draw(DrawingContext& context)
{
  light->set_color(color);
  light->set_blend(Blend::ADD);
  light->set_angle(angle);
  light->draw(context.light(), m_col.m_bbox.p1(), 0);

  //lightcone->set_angle(angle);
  //lightcone->draw(context.color(), position, 0);

  lights->set_angle(angle);
  lights->draw(context.color(), m_col.m_bbox.p1(), 0);

  base->set_angle(angle);
  base->draw(context.color(), m_col.m_bbox.p1(), 0);

  center->draw(context.color(), m_col.m_bbox.p1(), 0);

  lightcone->set_angle(angle);
  lightcone->draw(context.color(), m_col.m_bbox.p1(), LAYER_FOREGROUND1 + 10);
}

HitResponse
Spotlight::collision(GameObject& other, const CollisionHit& hit_)
{
  return FORCE_MOVE;
}

/* EOF */
