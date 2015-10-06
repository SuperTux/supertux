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
#include "util/gettext.hpp"
#include "util/reader.hpp"

Spotlight::Spotlight(const Reader& lisp) :
  position(),
  angle(0.0f),
  center(),
  base(),
  lights(),
  light(),
  lightcone(),
  color(1.0f, 1.0f, 1.0f)
{
  lisp.get("x", position.x);
  lisp.get("y", position.y);

  lisp.get("angle", angle);

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

ObjectSettings
Spotlight::get_settings() {
  ObjectSettings result(_("Spotlight"));
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Name"), &name));

  return result;
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
  light->draw(context, position, 0);

  //lightcone->set_angle(angle);
  //lightcone->draw(context, position, 0);

  context.set_target(DrawingContext::NORMAL);

  lights->set_angle(angle);
  lights->draw(context, position, 0);

  base->set_angle(angle);
  base->draw(context, position, 0);

  center->draw(context, position, 0);

  lightcone->set_angle(angle);
  lightcone->draw(context, position, LAYER_FOREGROUND1 + 10);

  context.pop_target();
}

/* EOF */
