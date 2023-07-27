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

#include "editor/editor.hpp"
#include "math/random.hpp"
#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "supertux/flip_level_transformer.hpp"
#include "supertux/sector.hpp"

static const float CRACKTIME = 0.3f;
static const float FALLTIME = 0.8f;
static const float RESPAWNTIME = 5.f;
static const float FADETIME = 0.5f;
static const float DELAY_IF_TUX = 0.001f;

SkullTile::SkullTile(const ReaderMapping& mapping) :
  MovingSprite(mapping, "images/objects/skull_tile/skull_tile.sprite", LAYER_TILES, COLGROUP_STATIC),
  physic(),
  timer(),
  hit(false),
  falling(false),
  m_revive_timer(),
  m_respawn(),
  m_alpha(1.f),
  m_original_pos(m_col.get_pos())
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
  if(!Editor::is_active())
  {
    // shaking
    if (timer.get_timegone() > CRACKTIME) {
      pos.x += static_cast<float>(graphicsRandom.rand(-3, 3));
    }
  }
  m_sprite->set_alpha(m_alpha);
  m_sprite->draw(context.color(), pos, m_layer, m_flip);
}

void
SkullTile::update(float dt_sec)
{
  if (falling) {
    if (m_revive_timer.check())
    {
      if (Sector::current() && Sector::get().is_free_of_movingstatics(m_col.m_bbox.grown(-1.f)))
      {
        m_alpha = 0.f;
        m_revive_timer.stop();
        falling = false;
        m_respawn.reset(new FadeHelper(&m_alpha, FADETIME, 1.f));
        physic.enable_gravity(false);
        m_col.set_pos(m_original_pos);
        physic.set_velocity(Vector(0.0f, 0.0f));
        m_col.set_movement(Vector(0.0f, 0.0f));
      }
      else
      {
        m_revive_timer.start(DELAY_IF_TUX);
      }
    }
    m_col.set_movement(physic.get_movement(dt_sec));
  } else if (hit) {
	  set_action("mad", -1);
    if (timer.check()) {
      falling = true;
      physic.enable_gravity(true);
      timer.stop();
      m_revive_timer.start(RESPAWNTIME);
    } else if (!timer.started()) {
      timer.start(FALLTIME);
    }
  } else {
    set_action("normal", -1);
    timer.stop();
  }
  hit = false;

  if (m_respawn && !m_respawn->completed())
    m_respawn->update(dt_sec);
}

void
SkullTile::on_flip(float height)
{
  MovingSprite::on_flip(height);
  FlipLevelTransformer::transform_flip(m_flip);
}

/* EOF */
