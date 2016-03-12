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
#include "supertux/object_factory.hpp"
#include "util/reader_mapping.hpp"

Spotlight::Spotlight(const ReaderMapping& lisp) :
  angle(),
  center(),
  base(),
  lights(),
  light(),
  lightcone(),
  color(1.0f, 1.0f, 1.0f)
{
  group = COLGROUP_DISABLED;

  if (!lisp.get("x", bbox.p1.x)) bbox.p1.x = 0;
  if (!lisp.get("y", bbox.p1.y)) bbox.p1.y = 0;
  bbox.set_size(32, 32);

  if (!lisp.get("angle", angle)) angle = 0.0f;

  std::vector<float> vColor;
  if( lisp.get( "color", vColor ) ){
    color = Color( vColor );
  }

  center    = SpriteManager::current()->create("images/objects/spotlight/spotlight_center.sprite");
  base      = SpriteManager::current()->create("images/objects/spotlight/spotlight_base.sprite");
  lights    = SpriteManager::current()->create("images/objects/spotlight/spotlight_lights.sprite");
  lightcone = SpriteManager::current()->create("images/objects/spotlight/lightcone.sprite");
  light     = SpriteManager::current()->create("images/objects/spotlight/light.sprite");

}

Spotlight::~Spotlight()
{
}

void
Spotlight::save(lisp::Writer& writer) {
  GameObject::save(writer);
  writer.write("x", position.x);
  writer.write("y", position.y);
  writer.write("color", color.toVector(false));
}

void
Spotlight::update(float delta)
{
  angle += delta * 50.0f;
}

void
Spotlight::draw(DrawingContext& context)
{
  context.push_target();
  context.set_target(DrawingContext::LIGHTMAP);

  light->set_color(color);
  light->set_blend(Blend(GL_SRC_ALPHA, GL_ONE));
  light->set_angle(angle);
  light->draw(context, bbox.p1, 0);

  //lightcone->set_angle(angle);
  //lightcone->draw(context, position, 0);

  context.set_target(DrawingContext::NORMAL);

  lights->set_angle(angle);
  lights->draw(context, bbox.p1, 0);

  base->set_angle(angle);
  base->draw(context, bbox.p1, 0);

  center->draw(context, bbox.p1, 0);

  lightcone->set_angle(angle);
  lightcone->draw(context, bbox.p1, LAYER_FOREGROUND1 + 10);

  context.pop_target();
}

HitResponse
Spotlight::collision(GameObject& other, const CollisionHit& hit_)
{
  return FORCE_MOVE;
}

/* EOF */
