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

Candle::Candle(const lisp::Lisp& lisp)
  : MovingSprite(lisp, "images/objects/candle/candle.sprite", LAYER_BACKGROUNDTILES+1, COLGROUP_DISABLED), burning(true),
    candle_light_1("images/objects/candle/candle-light-1.png"),
    candle_light_2("images/objects/candle/candle-light-2.png")
{
  lisp.get("name", name);
  lisp.get("burning", burning);

  if (burning) {
    sprite->set_action("on");
  } else {
    sprite->set_action("off");
  }

}

void
Candle::draw(DrawingContext& context)
{
  // draw regular sprite
  sprite->draw(context, get_pos(), layer);

  // draw on lightmap
  if (burning) {
    Vector pos = get_pos() + (bbox.get_size() - candle_light_1.get_size()) / 2;
    context.push_target();
    context.set_target(DrawingContext::LIGHTMAP);
    // draw approx. 1 in 10 frames darker. Makes the candle flicker
    if (systemRandom.rand(10) != 0) {
      context.draw_surface(&candle_light_1, pos, layer);
    } else {
      context.draw_surface(&candle_light_2, pos, layer);
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
  Scripting::Candle* interface = new Scripting::Candle(this);
  expose_object(vm, table_idx, interface, name, true);
}

void
Candle::unexpose(HSQUIRRELVM vm, SQInteger table_idx)
{
  if (name.empty()) return;
  Scripting::unexpose_object(vm, table_idx, name);
}

void
Candle::puff_smoke()
{
  Vector ppos = bbox.get_middle();
  Vector pspeed = Vector(0, -150);
  Vector paccel = Vector(0,0);
  Sector::current()->add_object(new SpriteParticle("images/objects/particles/smoke.sprite", "default", ppos, ANCHOR_MIDDLE, pspeed, paccel, LAYER_BACKGROUNDTILES+2));
}

bool
Candle::get_burning()
{
  return burning;
}

void
Candle::set_burning(bool burning)
{
  if (this->burning == burning) return;
  this->burning = burning;
  if (burning) {
    sprite->set_action("on");
    puff_smoke();
  } else {
    sprite->set_action("off");
    puff_smoke();
  }
}

IMPLEMENT_FACTORY(Candle, "candle");

/* EOF */
