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

#include "object/candle.hpp"

#include <simplesquirrel/class.hpp>
#include <simplesquirrel/vm.hpp>

#include "math/random.hpp"
#include "object/sprite_particle.hpp"
#include "sprite/sprite.hpp"
#include "supertux/flip_level_transformer.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"

Candle::Candle(const ReaderMapping& mapping) :
  MovingSprite(mapping, "images/objects/candle/candle.sprite", LAYER_BACKGROUNDTILES+1, COLGROUP_DISABLED),
  burning(true),
  flicker(true),
  lightcolor(1.0f, 1.0f, 1.0f),
  candle_light_1(m_sprite->create_linked_sprite("light-1")),
  candle_light_2(m_sprite->create_linked_sprite("light-2"))
{
  mapping.get("burning", burning, true);
  mapping.get("flicker", flicker, true);
  mapping.get("layer", m_layer); // Backwards compatibility

  std::vector<float> vColor;
  if (!mapping.get("color", vColor)) vColor = {1.f, 1.f, 1.f};

  //std::cout << vColor[0] << vColor[1] << vColor[2] << vColor[3] << std::endl;
  lightcolor = Color(vColor);

  candle_light_1->set_blend(Blend::ADD);
  candle_light_2->set_blend(Blend::ADD);

  // Change the light color if defined.
  if (lightcolor.greyscale() < 1.f)
  {
    candle_light_1->set_color(lightcolor);
    candle_light_2->set_color(lightcolor);

    set_action(burning ? "on-white" : "off-white");
  }
  else
  {
    set_action(burning ? "on" : "off");
  }
}

MovingSprite::LinkedSprites
Candle::get_linked_sprites()
{
  return {
    { "light-1", candle_light_1 },
    { "light-2", candle_light_2 }
  };
}

void
Candle::on_sprite_update()
{
  MovingSprite::on_sprite_update();

  candle_light_1->set_blend(Blend::ADD);
  candle_light_2->set_blend(Blend::ADD);

  // Change the light color if defined.
  if (lightcolor.greyscale() < 1.f)
  {
    candle_light_1->set_color(lightcolor);
    candle_light_2->set_color(lightcolor);
  }
}

void
Candle::after_editor_set()
{
  MovingSprite::after_editor_set();

  // Change the light color if defined.
  if (lightcolor.greyscale() < 1.f)
    set_action(burning ? "on-white" : "off-white");
  else
    set_action(burning ? "on" : "off");
}

ObjectSettings
Candle::get_settings()
{
  ObjectSettings result = MovingSprite::get_settings();

  result.add_bool(_("Burning"), &burning, "burning", true);
  result.add_bool(_("Flicker"), &flicker, "flicker", true);
  result.add_color(_("Color"), &lightcolor, "color", Color::WHITE);

  result.reorder({"burning", "flicker", "name", "sprite", "color", "z-pos", "x", "y"});

  return result;
}

void
Candle::draw(DrawingContext& context)
{
  // Draw regular sprite.
  MovingSprite::draw(context);

  // Draw on lightmap.
  if (burning) {
    // Vector pos = get_pos() + (bbox.get_size() - candle_light_1->get_size()) / 2;
    // draw approx. 1 in 10 frames darker. Makes the candle flicker.
    if (graphicsRandom.rand(10) != 0 || !flicker) {
      // context.color().draw_surface(candle_light_1, pos, layer);
      candle_light_1->draw(context.light(), m_col.m_bbox.get_middle(), m_layer);
    } else {
      // context.color().draw_surface(candle_light_2, pos, layer);
      candle_light_2->draw(context.light(), m_col.m_bbox.get_middle(), m_layer);
    }
  }
}

HitResponse
Candle::collision(MovingObject&, const CollisionHit& )
{
  return FORCE_MOVE;
}

void
Candle::puff_smoke()
{
  Vector ppos = m_col.m_bbox.get_middle();
  Vector pspeed = Vector(0, -150);
  Vector paccel = Vector(0,0);
  Sector::get().add<SpriteParticle>(m_sprite->get_linked_sprite("smoke"),
                                         ppos, ANCHOR_MIDDLE,
                                         pspeed, paccel,
                                         LAYER_BACKGROUNDTILES+2);
}

void
Candle::set_burning(bool burning_)
{
  if (burning == burning_) return;
  burning = burning_;
  if (lightcolor.greyscale() < 1.f)
    set_action(burning_ ? "on-white" : "off-white");
  else
    set_action(burning_ ? "on" : "off");

  // Puff smoke for flickering light sources only.
  if (flicker) puff_smoke();
}

void
Candle::on_flip(float height)
{
  MovingSprite::on_flip(height);
  FlipLevelTransformer::transform_flip(m_flip);
}


void
Candle::register_class(ssq::VM& vm)
{
  ssq::Class cls = vm.addAbstractClass<Candle>("Candle", vm.findClass("MovingSprite"));

  cls.addFunc("puff_smoke", &Candle::puff_smoke);
  cls.addFunc("get_burning", &Candle::get_burning);
  cls.addFunc("set_burning", &Candle::set_burning);

  cls.addVar("burning", &Candle::get_burning, &Candle::set_burning);
}

/* EOF */
