//  $Id$
//
//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include <config.h>

#include "firefly.hpp"
#include "resources.hpp"
#include "sprite/sprite_manager.hpp"
#include "video/drawing_context.hpp"
#include "player.hpp"
#include "object_factory.hpp"
#include "game_session.hpp"
#include "sector.hpp"
#include "random_generator.hpp"
#include "object/sprite_particle.hpp"

Firefly::Firefly(const lisp::Lisp& lisp)
       : MovingSprite(lisp, "images/objects/resetpoints/default-resetpoint.sprite", LAYER_TILES, COLGROUP_TOUCHABLE), activated(false)
{
  initial_position = get_pos();
  if( !lisp.get( "sprite", sprite_name ) ){
    reactivate();
    return;
  }
  if( sprite_name == "" ){
    sprite_name = "images/objects/resetpoints/default-resetpoint.sprite";
    reactivate();
    return;
  }
  //Replace sprite
  sprite = sprite_manager->create( sprite_name );
  bbox.set_size(sprite->get_current_hitbox_width(), sprite->get_current_hitbox_height());
  reactivate();
}

void
Firefly::reactivate()
{
  if(GameSession::current()->get_reset_point_pos() == initial_position){
    // TODO: && GameSession::current()->get_reset_point_sectorname() ==  <sector this firefly is in>
    // GameSession::current()->get_current_sector()->get_name() is not yet initialized.
    // Worst case a resetpoint in a different sector at the same position as the real
    // resetpoint the player is spawning is set to ringing, too. Until we can check the sector, too, dont set
    // activated = true; here.
    sprite->set_action("ringing");
  }
}

void
Firefly::write(lisp::Writer& writer)
{
  writer.start_list("firefly");
  writer.write("x", bbox.p1.x);
  writer.write("y", bbox.p1.y);
  writer.end_list("firefly");
}

HitResponse
Firefly::collision(GameObject& other, const CollisionHit& )
{
  if(activated)
    return ABORT_MOVE;

  Player* player = dynamic_cast<Player*> (&other);
  if(player) {
    activated = true;
// spawn some particles
// TODO: provide convenience function in MovingSprite or MovingObject?
       for (int i = 0; i < 5; i++) {
         Vector ppos = bbox.get_middle();
         float angle = systemRandom.randf(-M_PI_2, M_PI_2);
         float velocity = systemRandom.randf(450, 900);
         float vx = sin(angle)*velocity;
         float vy = -cos(angle)*velocity;
         Vector pspeed = Vector(vx, vy);
         Vector paccel = Vector(0, 1000);
         Sector::current()->add_object(new SpriteParticle("images/objects/particles/reset.sprite", "default", ppos, ANCHOR_MIDDLE, pspeed, paccel, LAYER_OBJECTS-1));
       }
    // TODO play sound
    sprite->set_action("ringing");
    GameSession::current()->set_reset_point(Sector::current()->get_name(),
        initial_position);
  }

  return ABORT_MOVE;
}

IMPLEMENT_FACTORY(Firefly, "firefly");
