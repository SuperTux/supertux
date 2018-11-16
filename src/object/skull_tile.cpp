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

#include "object/skull_tile.hpp"

#include "math/random.hpp"
#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "supertux/sector.hpp"

static const float CRACKTIME = 0.3f;
static const float FALLTIME = 0.8f;

SkullTile::SkullTile(const ReaderMapping& mapping) :
  MovingSprite(mapping, "images/objects/skull_tile/skull_tile.sprite", LAYER_TILES, COLGROUP_STATIC),
  physic(),
  timer(),
  hit(false),
  falling(false)
{
}

HitResponse
SkullTile::collision(GameObject& other, const CollisionHit& )
{
  auto player = dynamic_cast<Player*> (&other);
  if (player)
    hit = true;

  return FORCE_MOVE;
}

void
SkullTile::draw(DrawingContext& context)
{
  Vector pos = get_pos();
  // shaking
  if (timer.get_timegone() > CRACKTIME) {
    pos.x += static_cast<float>(graphicsRandom.rand(-3, 3));
  }

  m_sprite->draw(context.color(), pos, m_layer);
}

void
SkullTile::update(float dt_sec)
{
  if (falling) {
    m_col.m_movement = physic.get_movement(dt_sec);
    if (!Sector::get().inside(m_col.m_bbox)) {
      remove_me();
      return;
    }
  } else if (hit) {
    if (timer.check()) {
      falling = true;
      physic.enable_gravity(true);
      timer.stop();
    } else if (!timer.started()) {
      timer.start(FALLTIME);
    }
  } else {
    timer.stop();
  }
  hit = false;
}

/* EOF */
