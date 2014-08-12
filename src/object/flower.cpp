//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#include "audio/sound_manager.hpp"
#include "object/flower.hpp"
#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"

Flower::Flower(BonusType _type) :
  type(_type),
  sprite(),
  drawing_effect(NO_EFFECT),
  light(1.0f,1.0f,1.0f),
  lightsprite(sprite_manager->create("images/objects/lightmap_light/lightmap_light-small.sprite"))
{
  bbox.set_size(32, 32);
  lightsprite->set_blend(Blend(GL_SRC_ALPHA, GL_ONE));

  if(type == FIRE_BONUS) {
    sprite = sprite_manager->create("images/powerups/fireflower/fireflower.sprite");
    sound_manager->preload("sounds/fire-flower.wav");
    lightsprite->set_color(Color(0.3f, 0.0f, 0.0f));
  }
  else if(type == ICE_BONUS) {
    sprite = sprite_manager->create("images/powerups/iceflower/iceflower.sprite");
    sound_manager->preload("sounds/fire-flower.wav");
    lightsprite->set_color(Color(0.0f, 0.1f, 0.2f));
  } else {
    assert(false);
  }

  set_group(COLGROUP_TOUCHABLE);
}

Flower::~Flower()
{
}

void
Flower::update(float )
{
}

void
Flower::draw(DrawingContext& context)
{
  //Draw the Sprite.
  sprite->draw(context, get_pos(), LAYER_OBJECTS, drawing_effect);
  //Draw the light when dark
  context.get_light( get_bbox().get_middle(), &light );
  if (light.red + light.green + light.blue < 3.0){
    context.push_target();
    context.set_target(DrawingContext::LIGHTMAP);
    lightsprite->draw(context, get_bbox().get_middle(), 0);
    context.pop_target();
  }
}

HitResponse
Flower::collision(GameObject& other, const CollisionHit& )
{
  Player* player = dynamic_cast<Player*>(&other);
  if(!player)
    return ABORT_MOVE;

  if(!player->add_bonus(type, true))
    return FORCE_MOVE;

  sound_manager->play("sounds/fire-flower.wav");
  remove_me();
  return ABORT_MOVE;
}

/* EOF */
