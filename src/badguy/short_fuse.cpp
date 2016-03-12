//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//  Copyright (C) 2010 Florian Forster <supertux at octo.it>
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

#include "badguy/bomb.hpp"
#include "badguy/short_fuse.hpp"
#include "object/bullet.hpp"
#include "object/explosion.hpp"
#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"
#include "util/reader.hpp"
#include "util/log.hpp"
#include "util/reader_mapping.hpp"

#define EXPLOSION_FORCE 1000.0f

ShortFuse::ShortFuse(const ReaderMapping& reader) :
  WalkingBadguy(reader, "images/creatures/short_fuse/short_fuse.sprite", "left", "right")
{
  walk_speed = 100;
  max_drop_height = -1;

  //Check if we need another sprite
  if( !reader.get( "sprite", sprite_name ) ){
    return;
  }
  if (sprite_name.empty()) {
    sprite_name = "images/creatures/short_fuse/short_fuse.sprite";
    return;
  }
  //Replace sprite
  sprite = SpriteManager::current()->create( sprite_name );
}

void
ShortFuse::explode()
{
  if (!is_valid())
    return;

  auto explosion = std::make_shared<Explosion>(get_bbox ().get_middle());

  explosion->hurts(false);
  explosion->pushes(true);
  Sector::current()->add_object(explosion);

  run_dead_script();
  remove_me();
}

bool
ShortFuse::collision_squished(GameObject& obj)
{
  if (!is_valid ())
    return true;

  Player* player = dynamic_cast<Player*>(&obj);
  if(player)
    player->bounce(*this);

  explode ();

  return true;
}

HitResponse
ShortFuse::collision_player (Player& player, const CollisionHit&)
{
  player.bounce (*this);
  explode ();
  return ABORT_MOVE;
}

HitResponse
ShortFuse::collision_bullet (Bullet& bullet, const CollisionHit& )
{
  // All bullets cause the unstable short fuse to explode
  bullet.remove_me();
  explode();
  return ABORT_MOVE;
}

void
ShortFuse::kill_fall()
{
  explode ();
}

void
ShortFuse::ignite()
{
  kill_fall();
}

/* vim: set sw=2 sts=2 et : */
/* EOF */
