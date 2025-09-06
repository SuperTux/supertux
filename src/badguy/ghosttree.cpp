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
static const float SUCK_TARGET_SPREAD = 8;

GhostTree::GhostTree(const ReaderMapping& mapping) :
  Boss(mapping, "images/creatures/ghosttree/ghosttree.sprite", LAYER_OBJECTS - 10),
  m_state(STATE_INIT),
  m_attack(ATTACK_RED),
  m_state_timer(),
  m_willo_spawn_y(0),
  m_willo_radius(200),
  m_willo_speed(1.8f),
  m_willo_to_spawn(9),
  m_next_willo(ATTACK_RED),
  m_willowisps(),
  m_root_attack()
{
  mapping.get("hud-icon", m_hud_icon, "images/creatures/ghosttree/hudlife.png");
  m_hud_head = Surface::from_file(m_hud_icon);

  set_colgroup_active(COLGROUP_TOUCHABLE);
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
GhostTree::get_player_pos()
{
  auto player = get_nearest_player();
  if (player) {
    return player->get_pos();
  } else {
    return m_col.m_bbox.get_middle();
  }
}

void
GhostTree::active_update(float dt_sec)
{
  Boss::boss_update(dt_sec);
  switch (m_state) {
    case STATE_INIT:
      if (m_state_timer.check()) {
        spawn_willowisp(m_next_willo);
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
          spawn_willowisp(m_next_willo);
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
      break;
    default:
      break;
  }
}

bool
GhostTree::suck_now(const Color& color) const {
  switch (m_attack) {
    case ATTACK_RED:
      return color.red == 1.0;
    case ATTACK_GREEN:
      return color.green == 1.0;
    case ATTACK_BLUE:
      return color.blue == 1.0;
    case ATTACK_PINCH:
      return true;
    default:
      return false;
  }
}

void
GhostTree::rotate_willo_color() {
  m_next_willo = static_cast<AttackType>((static_cast<int>(m_next_willo) + 1) % 3);
}

void
GhostTree::spawn_willowisp(AttackType color) {
  Vector pos(m_col.m_bbox.get_width() / 2,
  m_col.m_bbox.get_height() / 2 + (m_flip == NO_FLIP ? (m_willo_spawn_y + WILLOWISP_TOP_OFFSET) :
                                                      -(m_willo_spawn_y + WILLOWISP_TOP_OFFSET + 32.0f)));
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
      set_action("idle");
      m_state_timer.start(0.1);
      break;
    case STATE_SCREAM:
      set_action("scream");
      SoundManager::current()->play("sounds/tree_howling.ogg", get_pos());
      m_state_timer.start(2);
      break;
    case STATE_IDLE:
      set_action(m_attack == ATTACK_PINCH ? "idle-pinch" : "idle");
      start_attack(true);
      break;
    case STATE_SUCKING:
      SoundManager::current()->play("sounds/tree_suck.ogg", get_pos());
      for (const auto& willo : m_willowisps) {
        if (suck_now(willo->get_color())) {
          willo->start_sucking(
            m_col.m_bbox.get_middle() + SUCK_TARGET_OFFSET
            + Vector(gameRandom.randf(-SUCK_TARGET_SPREAD, SUCK_TARGET_SPREAD),
                     gameRandom.randf(-SUCK_TARGET_SPREAD, SUCK_TARGET_SPREAD)));
        }
      }
      break;
    case STATE_ATTACKING:
      set_action(m_attack == ATTACK_PINCH ? "scream-pinch" : "scream");
      SoundManager::current()->play("sounds/tree_howling.ogg", get_pos());
      start_attack(false);
      break;
    case STATE_RECHARGING:
      set_action(m_attack == ATTACK_PINCH ? "charge-pinch" : "charge");
      m_state_timer.start(1);
      m_willo_to_spawn = m_attack == ATTACK_PINCH ? 9 : 3;
      break;
    case STATE_DEAD:
      SoundManager::current()->play("sounds/tree_howling.ogg", get_pos());
      set_action("busted");
      run_dead_script();
      break;
    default:
      break;
  }
  m_state = new_state;
}

void
GhostTree::start_attack(bool main_root)
{
  const float middle = m_col.m_bbox.get_middle().x;
  const float base = m_col.m_bbox.get_bottom() + 96;
  if (main_root) {
    m_root_attack.reset(new GhostTreeAttackMain(Vector(middle, base)));
    return;
  }

  Vector player_pos = get_player_pos();
  switch (m_attack) {
    case ATTACK_RED:
      if (player_pos.x > middle) {
        m_root_attack.reset(new GhostTreeAttackRed(base, middle - 512, middle + 512));
      } else {
        m_root_attack.reset(new GhostTreeAttackRed(base, middle + 512, middle - 512));
      }
      break;
    case ATTACK_GREEN:
      m_root_attack.reset(new GhostTreeAttackGreen(Vector(player_pos.x, base)));
      break;
    case ATTACK_BLUE:
      m_root_attack.reset(new GhostTreeAttackBlue(Vector(player_pos.x, base)));
      break;
    case ATTACK_PINCH:
    default:
      m_root_attack.reset(new GhostTreeAttackPinch(Vector(player_pos.x, base), middle - 512, middle + 512));
      break;
  }
}

void
GhostTree::willowisp_died(TreeWillOWisp* willowisp)
{
  if ((m_state == STATE_SUCKING) && (willowisp->was_sucked)) {
    set_state(STATE_ATTACKING);
  }
  m_willowisps.erase(std::find_if(m_willowisps.begin(), m_willowisps.end(),
                                  [willowisp](const GameObject* lhs)
                                  {
                                    return lhs == willowisp;
                                  }));
}

bool
GhostTree::collides(MovingObject& other, const CollisionHit& ) const
{
  if (m_state != STATE_RECHARGING) return false;
  if (dynamic_cast<Player*>(&other)) return true;
  return false;
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
  
  SoundManager::current()->play(m_attack == ATTACK_BLUE ? "sounds/tree_pinch.ogg" : "sounds/gulp.wav", get_pos());

  --m_lives;
  if (m_lives <= 0) {
    set_state(STATE_DEAD);
    return true;
  }
  
  if (m_attack == ATTACK_PINCH) {
    while (m_willo_to_spawn--) {
      spawn_willowisp(m_next_willo);
      rotate_willo_color();
    }
  } else {
    while (m_willo_to_spawn--) {
      spawn_willowisp(m_attack);
    }
    m_attack = static_cast<AttackType>(static_cast<int>(m_attack) + 1);
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
