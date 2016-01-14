//  SuperTux - Lantern
//  Copyright (C) 2006 Wolfgang Becker <uafr@gmx.de>
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

#include "object/lantern.hpp"

#include <algorithm>

#include "audio/sound_manager.hpp"
#include "badguy/treewillowisp.hpp"
#include "badguy/willowisp.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/object_factory.hpp"
#include "util/reader_mapping.hpp"

Lantern::Lantern(const ReaderMapping& reader) :
  Rock(reader, "images/objects/lantern/lantern.sprite"),
  lightcolor(1.0f, 1.0f, 1.0f),
  lightsprite()
{
  //get color from lisp
  std::vector<float> vColor;
  if (reader.get("color", vColor)) {
    lightcolor = Color(vColor);
  } else {
    lightcolor = Color(0, 0, 0);
  }
  lightsprite = SpriteManager::current()->create("images/objects/lightmap_light/lightmap_light.sprite");
  lightsprite->set_blend(Blend(GL_SRC_ALPHA, GL_ONE));
  updateColor();
  SoundManager::current()->preload("sounds/willocatch.wav");
}

Lantern::Lantern(const Vector& pos) :
  Rock(pos, "images/objects/lantern/lantern.sprite"),
  lightcolor(0.0f, 0.0f, 0.0f),
  lightsprite(SpriteManager::current()->create("images/objects/lightmap_light/lightmap_light.sprite"))
{
  lightsprite->set_blend(Blend(GL_SRC_ALPHA, GL_ONE));
  updateColor();
  SoundManager::current()->preload("sounds/willocatch.wav");
}

Lantern::~Lantern()
{
}

void
Lantern::updateColor(){
  lightsprite->set_color(lightcolor);
  //Turn lantern off if light is black
  if(lightcolor.red == 0 && lightcolor.green == 0 && lightcolor.blue == 0){
    sprite->set_action("off");
  } else {
    sprite->set_action("normal");
    sprite->set_color(lightcolor);
  }
}

void
Lantern::draw(DrawingContext& context){
  //Draw the Sprite.
  MovingSprite::draw(context);
  //Let there be light.
  context.push_target();
  context.set_target(DrawingContext::LIGHTMAP);

  lightsprite->draw(context, bbox.get_middle(), 0);

  context.pop_target();
}

HitResponse Lantern::collision(GameObject& other, const CollisionHit& hit) {
  if (is_open()) {
    WillOWisp* wow = dynamic_cast<WillOWisp*>(&other);
    if (wow) {
      // collided with WillOWisp while grabbed and unlit
      SoundManager::current()->play("sounds/willocatch.wav");
      lightcolor = Color(0,1,0);
      updateColor();
      wow->vanish();
    }
    TreeWillOWisp* twow = dynamic_cast<TreeWillOWisp*>(&other);
    if (twow) {
      // collided with TreeWillOWisp while grabbed and unlit
      SoundManager::current()->play("sounds/willocatch.wav");
      lightcolor = twow->get_color();
      updateColor();
      twow->vanish();
    }
  }
  return Rock::collision(other, hit);
}

void
Lantern::grab(MovingObject& object, const Vector& pos, Direction dir)
{
  Rock::grab(object, pos, dir);

  // if lantern is not lit, draw it as opened
  if (is_open()) {
    sprite->set_action("off-open");
  }

}

void
Lantern::ungrab(MovingObject& object, Direction dir)
{
  // if lantern is not lit, it was drawn as opened while grabbed. Now draw it as closed again
  if (is_open()) {
    sprite->set_action("off");
  }

  Rock::ungrab(object, dir);
}

bool
Lantern::is_open() const
{
  return ((grabbed) && lightcolor.red == 0 && lightcolor.green == 0 && lightcolor.blue == 0);
}

void
Lantern::add_color(Color c) {
  lightcolor.red   = std::min(1.0f, lightcolor.red   + c.red);
  lightcolor.green = std::min(1.0f, lightcolor.green + c.green);
  lightcolor.blue  = std::min(1.0f, lightcolor.blue  + c.blue);
  lightcolor.alpha = std::min(1.0f, lightcolor.alpha + c.alpha);
  updateColor();
}

/* EOF */
