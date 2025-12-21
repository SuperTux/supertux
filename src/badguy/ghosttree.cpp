//  SuperTux - Boss "GhostTree"
//  Copyright (C) 2007 Matthias Braun <matze@braunis.de>
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

#include "badguy/ghosttree.hpp"

#include <algorithm>
#include <math.h>

#include "audio/sound_manager.hpp"
#include "badguy/ghosttree_attack.hpp"
#include "badguy/root.hpp"
#include "badguy/treewillowisp.hpp"
#include "math/random.hpp"
#include "object/lantern.hpp"
#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/flip_level_transformer.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"
#include "video/surface.hpp"

static const size_t WILLOWISP_COUNT = 10;
static const float WILLOWISP_TOP_OFFSET = -64;
static const Vector SUCK_TARGET_OFFSET = Vector(-16,-16);
static const float SUCK_TARGET_SPREAD = 16;

GhostTree::GhostTree(const ReaderMapping& mapping) :
  Boss(mapping, "images/creatures/ghosttree/ghosttree.sprite", LAYER_OBJECTS - 10,
       "images/creatures/mole/corrupted/core_glow/core_glow.sprite"),
  m_state(STATE_INIT),
  m_attack(ATTACK_NORMAL),
  m_state_timer(),
  m_willo_spawn_y(0),
  m_willo_radius(200),
  m_willo_speed(1.8f),
  m_willo_to_spawn(9),
  m_willo(ATTACK_RED),
  m_willowisps(),
  m_root_attack()
{
  mapping.get("hud-icon", m_hud_icon, "images/creatures/ghosttree/hudlife.png");
  m_hud_head = Surface::from_file(m_hud_icon);

  set_colgroup_active(COLGROUP_TOUCHABLE);
  SoundManager::current()->preload("sounds/tree_hit.ogg");
  SoundManager::current()->preload("sounds/tree_howling.ogg");
  SoundManager::current()->preload("sounds/tree_suck.ogg");
  SoundManager::current()->preload("sounds/tree_pinch.ogg");
  SoundManager::current()->preload("sounds/gulp.wav");
  SoundManager::current()->preload("sounds/explosion.wav"); // for green and pinch root
  SoundManager::current()->preload("sounds/dartfire.wav"); // for blue and pinch root
  
  set_state(STATE_INIT);
}

GhostTree::~GhostTree()
{

}

void
GhostTree::activate()
{
}

Vector
GhostTree::get_attack_pos() const
{
  const float middle = m_col.m_bbox.get_middle().x;
  const float base = m_col.m_bbox.get_bottom() + 96;

  // NOTE: This should never be the case when entering SPITTING state.
  if (m_attack == ATTACK_NORMAL)
    return Vector(middle, base);

  Player* player = get_nearest_player();
  Vector player_pos = player ? player->get_pos() : get_bbox().get_middle();

  switch (m_attack)
  {
    case ATTACK_RED:
      if (player_pos.x > middle)
        return Vector(middle - 512, base);
      else
        return Vector(middle + 512, base);

    default:
      return Vector(player_pos.x, base);
  }
}

void
GhostTree::active_update(float dt_sec)
{
  Boss::boss_update(dt_sec);
  switch (m_state) {
    case STATE_INIT:
      if (m_state_timer.check()) {
        spawn_willowisp(m_willo);
        rotate_willo_color();
        m_state_timer.start(0.1);
        --m_willo_to_spawn;
        if (m_willo_to_spawn <= 0) {
          set_state(STATE_SCREAM);
        }
      }
      break;

    case STATE_SCREAM:
      if (m_state_timer.check()) {
        set_state(STATE_IDLE);
      }
      break;

    case STATE_IDLE:
      m_root_attack->active_update(dt_sec);
      if (m_root_attack->is_done()) {
        set_state(STATE_SUCKING);
      }
      break;

    case STATE_SUCKING:
      if (m_state_timer.check())
      {
        set_state(STATE_SPITTING);
        for (const auto& wisp : m_willowisps) {
          if (should_suck(wisp->get_color())) {
            wisp->start_sucking(m_attack_pos + Vector(gameRandom.randf(-SUCK_TARGET_SPREAD, SUCK_TARGET_SPREAD),
                                                      gameRandom.randf(-SUCK_TARGET_SPREAD, SUCK_TARGET_SPREAD)),
                                0.5f);
          }
        }
      }
      break;

    case STATE_SPITTING:
      break;

    case STATE_ATTACKING:
      m_root_attack->active_update(dt_sec);
      if (m_root_attack->is_done()) {
        set_state(STATE_RECHARGING);
      }
      break;

    case STATE_RECHARGING:
      if (m_state_timer.check()) {
        if (m_attack == ATTACK_PINCH) {
          spawn_willowisp(m_willo);
          rotate_willo_color();
          m_state_timer.start(0.3);
        } else {
          spawn_willowisp(m_attack);
          m_state_timer.start(0.9);
        }

        --m_willo_to_spawn;
        if (m_willo_to_spawn <= 0) {
          set_state(STATE_IDLE);
        }
      }
      break;

    case STATE_DEAD:
      if (m_state_timer.check())
      {
        set_state(STATE_MUSIC_FADE_OUT);
      }
      break;

    case STATE_MUSIC_FADE_OUT:
      if (m_state_timer.check())
      {
        set_state(STATE_WISP_FLY_AWAY);
      }
      break;

    case STATE_WISP_FLY_AWAY:
      if (m_state_timer.check())
      {
        run_dead_script();
      }

    default:
      break;
  }
}

bool
GhostTree::should_suck(const Color& color) const {
  if (m_attack == ATTACK_PINCH)
    return true;

  switch (m_willo) {
    case ATTACK_RED:
      return color.red == 1.0;
    case ATTACK_GREEN:
      return color.green == 1.0;
    case ATTACK_BLUE:
      return color.blue == 1.0;
    default:
      return false;
  }
}

void
GhostTree::rotate_willo_color() {
  m_willo = static_cast<AttackType>(static_cast<int>(ATTACK_FIRST_SPECIAL)
                                    + ((static_cast<int>(m_willo) - static_cast<int>(ATTACK_FIRST_SPECIAL)
                                        + 1) % 3));
}

void
GhostTree::spawn_willowisp(AttackType color) {
  Vector pos(m_col.m_bbox.get_width() / 2,
  m_col.m_bbox.get_height() / 2 + (m_flip == NO_FLIP
                                   ? (m_willo_spawn_y + WILLOWISP_TOP_OFFSET)
                                   : -(m_willo_spawn_y + WILLOWISP_TOP_OFFSET + 32.0f)));
  auto& willowisp = Sector::get().add<TreeWillOWisp>(this, pos, 200 + m_willo_radius, m_willo_speed);
  m_willowisps.push_back(&willowisp);

  m_willo_spawn_y -= 40;
  if (m_willo_spawn_y < -160)
    m_willo_spawn_y = 0;

  m_willo_radius += 20;
  if (m_willo_radius > 120)
    m_willo_radius = 0;

  if (m_willo_speed == 1.8f) {
    m_willo_speed = 1.5f;
  } else {
    m_willo_speed = 1.8f;
  }
  
  switch (color) {
    case ATTACK_RED:
      willowisp.set_color(Color::RED);
      break;
    case ATTACK_GREEN:
      willowisp.set_color(Color::GREEN);
      break;
    case ATTACK_BLUE:
      willowisp.set_color(Color::BLUE);
      break;
    case ATTACK_PINCH:
      break;
    default:
      break;
  }
}

void
GhostTree::set_state(MyState new_state) {
  switch (new_state) {
    case STATE_INIT:
      m_glowing = false;
      set_action("idle");
      m_state_timer.start(0.1);
      break;

    case STATE_SCREAM:
      m_glowing = true;
      set_action("scream");
      SoundManager::current()->play("sounds/tree_howling.ogg", get_pos());
      m_state_timer.start(2);
      break;

    case STATE_IDLE:
      m_glowing = false;
      set_action(m_pinch_mode ? "idle-pinch" : "idle");
      m_attack = ATTACK_NORMAL;
      m_attack_pos = get_attack_pos();
      start_attack();
      break;

    case STATE_SUCKING:
      m_glowing = false;
      m_state_timer.stop();
      SoundManager::current()->play("sounds/tree_suck.ogg", get_pos());

      if (m_pinch_mode)
      {
        m_attack = ATTACK_PINCH;
      }
      else
      {
        m_attack = m_willo;
      }

      for (const auto& willo : m_willowisps) {
        if (should_suck(willo->get_color())) {
          willo->start_sucking(m_col.m_bbox.get_middle() + SUCK_TARGET_OFFSET
                               + Vector(gameRandom.randf(-SUCK_TARGET_SPREAD, SUCK_TARGET_SPREAD),
                                        gameRandom.randf(-SUCK_TARGET_SPREAD, SUCK_TARGET_SPREAD)),
                               2.5f);
        }
      }

      break;

    case STATE_SPITTING:
      m_glowing = true;
      set_action(m_pinch_mode ? "scream-pinch" : "scream");
      SoundManager::current()->play("sounds/tree_howling.ogg", get_pos());
      m_attack_pos = get_attack_pos();
      break;

    case STATE_ATTACKING:
      m_glowing = true;
      start_attack();
      break;

    case STATE_RECHARGING:
      m_glowing = true;
      set_action(m_pinch_mode ? "charge-pinch" : "charge");
      m_col.set_unisolid(true);
      m_state_timer.start(1);
      m_willo_to_spawn = m_pinch_mode ? 9 : 3;
      break;

    case STATE_DEAD:
      m_glowing = false;
      SoundManager::current()->play("sounds/tree_howling.ogg", get_pos());
      set_action("busted");
      m_state_timer.start(2.f);
      break;

    case STATE_MUSIC_FADE_OUT:
      SoundManager::current()->stop_music(3.f);
      m_state_timer.start(3.f + 2.f);
      break;

    case STATE_WISP_FLY_AWAY:
      for (TreeWillOWisp* wisp : m_willowisps)
        wisp->fly_away(get_bbox().get_middle());
      m_state_timer.start(3.f);
      break;

    default:
      break;
  }
  m_state = new_state;
}

void
GhostTree::start_attack()
{
  float middle = get_bbox().get_middle().x;
  switch (m_attack) {
    case ATTACK_NORMAL:
      m_root_attack.reset(new GhostTreeAttackMain(m_attack_pos));
      break;

    case ATTACK_RED:
    {
      if (m_attack_pos.x > middle)
        m_root_attack.reset(new GhostTreeAttackRed(m_attack_pos.y, m_attack_pos.x, m_attack_pos.x - (512 * 2)));
      else
        m_root_attack.reset(new GhostTreeAttackRed(m_attack_pos.y, m_attack_pos.x, m_attack_pos.x + (512 * 2)));

      break;
    }

    case ATTACK_GREEN:
      m_root_attack.reset(new GhostTreeAttackGreen(m_attack_pos));
      break;
    case ATTACK_BLUE:
      m_root_attack.reset(new GhostTreeAttackBlue(m_attack_pos));
      break;
    case ATTACK_PINCH:
    default:
      m_root_attack.reset(new GhostTreeAttackPinch(m_attack_pos, middle - 512, middle + 512));
      break;
  }
}

void
GhostTree::willowisp_suck_finished(TreeWillOWisp* willowisp)
{
  switch (m_state) {
    case STATE_SUCKING:
      if (willowisp->was_sucked())
        break;

      if (!m_state_timer.started())
        m_state_timer.start(0.5f);

      break;

    case STATE_SPITTING:
      set_state(STATE_ATTACKING);
      [[fallthrough]];

    case STATE_ATTACKING:
      willowisp->vanish();
      m_willowisps.erase(std::remove_if(m_willowisps.begin(), m_willowisps.end(),
                         [willowisp](const TreeWillOWisp* w) -> bool {
                           return w == willowisp;
                         }));
      break;

    default:
      break;
  }
}

bool
GhostTree::collides(MovingObject& other, const CollisionHit& hit) const
{
  return hit.bottom && m_state == STATE_RECHARGING && dynamic_cast<Player*>(&other) != nullptr;
}

HitResponse
GhostTree::collision(MovingObject& other, const CollisionHit& hit)
{
  if (m_state != STATE_RECHARGING) return ABORT_MOVE;
  return BadGuy::collision(other, hit);
}

bool
GhostTree::collision_squished(MovingObject& object)
{
  auto player = dynamic_cast<Player*>(&object);
  if (player) {
    player->bounce(*this);
  }
  
  SoundManager::current()->play("sounds/tree_hit.ogg", get_pos());

  --m_lives;
  if (m_lives <= 0) {
    set_state(STATE_DEAD);
    return true;
  }
  else if (m_lives <= m_pinch_lives)
  {
    m_pinch_mode = true;
  }
  
  if (m_pinch_mode) {
    while (m_willo_to_spawn--) {
      spawn_willowisp(m_willo);
      rotate_willo_color();
    }
  } else {
    while (m_willo_to_spawn--) {
      spawn_willowisp(m_attack);
    }
    rotate_willo_color();
    m_attack = static_cast<AttackType>(static_cast<int>(ATTACK_FIRST_SPECIAL)
                                       + ((static_cast<int>(m_attack) - static_cast<int>(ATTACK_FIRST_SPECIAL)
                                           + 1) % 3));
  }
  
  set_state(STATE_IDLE);
  return true;
}

std::vector<Direction>
GhostTree::get_allowed_directions() const
{
  return {};
}

void
GhostTree::on_flip(float height)
{
  BadGuy::on_flip(height);
  FlipLevelTransformer::transform_flip(m_flip);
}
