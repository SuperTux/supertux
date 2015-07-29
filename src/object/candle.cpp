//  SuperTux
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

#include "math/random_generator.hpp"
#include "object/candle.hpp"
#include "object/sprite_particle.hpp"
#include "scripting/candle.hpp"
#include "scripting/squirrel_util.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"

Candle::Candle(const ReaderMapping& lisp)
  : MovingSprite(lisp, "images/objects/candle/candle.sprite", LAYER_BACKGROUNDTILES+1, COLGROUP_DISABLED),
    burning(true),
    flicker(true),
    lightcolor(1.0f, 1.0f, 1.0f),
    candle_light_1(SpriteManager::current()->create("images/objects/candle/candle-light-1.sprite")),
    candle_light_2(SpriteManager::current()->create("images/objects/candle/candle-light-2.sprite"))
{

  if(!lisp.get("name", name))
  {
    log_debug << "Couldn't get \"name\" property for candle." << std::endl;
  }
  if(!lisp.get("burning", burning))
  {
    log_debug << "Couldn't get \"burning\" property for candle." << std::endl;
  }
  if(!lisp.get("flicker", flicker))
  {
    log_debug << "Couldn't get \"flicker\" property for candle." << std::endl;
  }
  //get color from lisp
  std::vector<float> vColor;
  if(lisp.get("color", vColor))
  {
    log_debug << "Couldn't get \"color\" property for candle." << std::endl;
  }

  //change the light color if defined
  if (vColor.size() >= 3) {
    lightcolor = Color(vColor);
    candle_light_1->set_blend(Blend(GL_SRC_ALPHA, GL_ONE));
    candle_light_2->set_blend(Blend(GL_SRC_ALPHA, GL_ONE));
    candle_light_1->set_color(lightcolor);
    candle_light_2->set_color(lightcolor);
    //the following allows the original candle appearance to be preserved
    candle_light_1->set_action("white");
    candle_light_2->set_action("white");
  }

  if (burning) {
    sprite->set_action("on");
  } else {
    sprite->set_action("off");
  }

}

void
Candle::save(lisp::Writer& writer) {
  MovingSprite::save(writer);
  writer.write("burning", burning);
  writer.write("flicker", flicker);
  writer.write("color", lightcolor.toVector(false));
}

void
Candle::draw(DrawingContext& context)
{
  // draw regular sprite
  sprite->draw(context, get_pos(), layer);

  // draw on lightmap
  if (burning) {
    //Vector pos = get_pos() + (bbox.get_size() - candle_light_1->get_size()) / 2;
    context.push_target();
    context.set_target(DrawingContext::LIGHTMAP);
    // draw approx. 1 in 10 frames darker. Makes the candle flicker
    if (gameRandom.rand(10) != 0 || !flicker) {
      //context.draw_surface(candle_light_1, pos, layer);
      candle_light_1->draw(context, bbox.get_middle(), 0);
    } else {
      //context.draw_surface(candle_light_2, pos, layer);
      candle_light_2->draw(context, bbox.get_middle(), 0);
    }
    context.pop_target();
  }
}

HitResponse
Candle::collision(GameObject&, const CollisionHit& )
{
  return FORCE_MOVE;
}

void
Candle::expose(HSQUIRRELVM vm, SQInteger table_idx)
{
  if (name.empty()) return;
  scripting::Candle* _this = new scripting::Candle(this);
  expose_object(vm, table_idx, _this, name, true);
}

void
Candle::unexpose(HSQUIRRELVM vm, SQInteger table_idx)
{
  if (name.empty()) return;
  scripting::unexpose_object(vm, table_idx, name);
}

void
Candle::puff_smoke()
{
  Vector ppos = bbox.get_middle();
  Vector pspeed = Vector(0, -150);
  Vector paccel = Vector(0,0);
  Sector::current()->add_object(std::make_shared<SpriteParticle>("images/objects/particles/smoke.sprite",
                                                                 "default",
                                                                 ppos, ANCHOR_MIDDLE,
                                                                 pspeed, paccel,
                                                                 LAYER_BACKGROUNDTILES+2));
}

bool
Candle::get_burning() const
{
  return burning;
}

void
Candle::set_burning(bool burning_)
{
  if (this->burning == burning_) return;
  this->burning = burning_;
  if (burning_) {
    sprite->set_action("on");
  } else {
    sprite->set_action("off");
  }
  //puff smoke for flickering light sources only
  if (flicker) puff_smoke();
}

/* EOF */
