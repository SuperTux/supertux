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
  //m_willowisp_timer(),
  m_willo_spawn_y(0),
  m_willo_radius(200),
  m_willo_speed(1.8f),
  m_willo_to_spawn(9),
  m_next_willo(ATTACK_RED),
  /*willo_color(0),*/
  //glow_sprite(SpriteManager::current()->create("images/creatures/ghosttree/ghosttree-glow.sprite")),
  /*colorchange_timer(),
  suck_timer(),
  root_timer(),
  treecolor(0),
  suck_lantern_color(),
  m_taking_life(),
  suck_lantern(nullptr),*/
  m_willowisps()
{
  mapping.get("hud-icon", m_hud_icon, "images/creatures/ghosttree/hudlife.png");
  m_hud_head = Surface::from_file(m_hud_icon);

  set_colgroup_active(COLGROUP_TOUCHABLE);
  SoundManager::current()->preload("sounds/tree_howling.ogg");
  SoundManager::current()->preload("sounds/tree_suck.ogg");

  set_state(STATE_INIT);
}

void
GhostTree::die()
{
  //TODO
  /*for (const auto& willo : willowisps) {
    willo->vanish();
  }

  if (m_lives <= 0) {
    mystate = STATE_DYING;
    set_action("dying", 1);
    glow_sprite->set_action("dying", 1);
    run_dead_script();
  }*/
}

void
GhostTree::activate()
{
  /*willowisp_timer.start(1.0f, true);
  colorchange_timer.start(13, true);
  root_timer.start(5, true);*/
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
      if (m_state_timer.check()) {
        set_state(STATE_SUCKING);
      }
      break;
    case STATE_SUCKING:
      break;
    case STATE_ATTACKING:
      if (m_state_timer.check()) {
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
  /*
  if (mystate == STATE_IDLE) {
    m_taking_life = false;
    if (colorchange_timer.check()) {
      SoundManager::current()->play("sounds/tree_howling.ogg", get_pos());
      suck_timer.start(3);
      treecolor = (treecolor + 1) % 3;

      Color col;
      switch (treecolor) {
        case 0: col = Color(1, 0, 0); break;
        case 1: col = Color(0, 1, 0); break;
        case 2: col = Color(0, 0, 1); break;
        case 3: col = Color(1, 1, 0); break;
        case 4: col = Color(1, 0, 1); break;
        case 5: col = Color(0, 1, 1); break;
        default: assert(false);
      }
      glow_sprite->set_color(col);
    }

    if (suck_timer.check()) {
      Color col = glow_sprite->get_color();
      SoundManager::current()->play("sounds/tree_suck.ogg", get_pos());
      for (const auto& willo : willowisps) {
        if (willo->get_color() == col) {
          willo->start_sucking(
            m_col.m_bbox.get_middle() + SUCK_TARGET_OFFSET
            + Vector(gameRandom.randf(-SUCK_TARGET_SPREAD, SUCK_TARGET_SPREAD),
                     gameRandom.randf(-SUCK_TARGET_SPREAD, SUCK_TARGET_SPREAD)));
        }
      }
      mystate = STATE_SUCKING;
    }

    if (willowisp_timer.check()) {
      if (willowisps.size() < WILLOWISP_COUNT) {
        Vector pos(m_col.m_bbox.get_width() / 2,
                   m_col.m_bbox.get_height() / 2 + (m_flip == NO_FLIP ? (willo_spawn_y + WILLOWISP_TOP_OFFSET) :
                                                                       -(willo_spawn_y + WILLOWISP_TOP_OFFSET + 32.0f)));
        auto& willowisp = Sector::get().add<TreeWillOWisp>(this, pos, 200 + willo_radius, willo_speed);
        willowisps.push_back(&willowisp);

        willo_spawn_y -= 40;
        if (willo_spawn_y < -160)
          willo_spawn_y = 0;

        willo_radius += 20;
        if (willo_radius > 120)
          willo_radius = 0;

        if (willo_speed == 1.8f) {
          willo_speed = 1.5f;
        } else {
          willo_speed = 1.8f;
        }

        do {
          willo_color = (willo_color + 1) % 3;
        } while(willo_color == treecolor);

        switch (willo_color) {
          case 0: willowisp.set_color(Color(1, 0, 0)); break;
          case 1: willowisp.set_color(Color(0, 1, 0)); break;
          case 2: willowisp.set_color(Color(0, 0, 1)); break;
          case 3: willowisp.set_color(Color(1, 1, 0)); break;
          case 4: willowisp.set_color(Color(1, 0, 1)); break;
          case 5: willowisp.set_color(Color(0, 1, 1)); break;
          default: assert(false);
        }
      }
    }

  } else if (mystate == STATE_SWALLOWING) {
    if (suck_lantern) {
      // Suck in the lantern.
      assert (suck_lantern);
      Vector pos = suck_lantern->get_pos();
      Vector delta = m_col.m_bbox.get_middle() + SUCK_TARGET_OFFSET - pos;
      if (glm::length(delta) < 1) {
        suck_lantern->ungrab(*this, Direction::RIGHT);
        suck_lantern->remove_me();
        suck_lantern = nullptr;
        set_action("swallow", 1);
      } else {
        pos += glm::normalize(delta);
        suck_lantern->grab(*this, pos, Direction::RIGHT);
      }
    } else {
      // Wait until the lantern is swallowed completely.
      if (m_sprite->animation_done()) {
        if (is_color_deadly(suck_lantern_color)) {
          if (!m_taking_life) {
            m_lives--;
            m_taking_life = true;
          }
          die();
        }
        if (m_lives > 0) {
          set_action("default");
          mystate = STATE_IDLE;
          spawn_lantern();
        }
      }
    }
  }*/
}

bool GhostTree::suck_now(const Color& color) const {
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
      willowisp.set_color(Color(1, 0, 0));
      break;
    case ATTACK_GREEN:
      willowisp.set_color(Color(0, 1, 0));
      break;
    case ATTACK_BLUE:
      willowisp.set_color(Color(0, 0, 1));
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
      std::cout<<"init"<<std::endl;
      set_action("idle");
      m_state_timer.start(0.1);
      break;
    case STATE_SCREAM:
      std::cout<<"scream"<<std::endl;
      set_action("scream");
      SoundManager::current()->play("sounds/tree_howling.ogg", get_pos());
      m_state_timer.start(2);
      break;
    case STATE_IDLE:
      std::cout<<"idle"<<std::endl;
      set_action(m_attack == ATTACK_PINCH ? "idle-pinch" : "idle");
      m_state_timer.start(5);
      break;
    case STATE_SUCKING:
      std::cout<<"sucking"<<std::endl;
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
      std::cout<<"attacking"<<std::endl;
      set_action(m_attack == ATTACK_PINCH ? "scream-pinch" : "scream");
      SoundManager::current()->play("sounds/tree_howling.ogg", get_pos());
      m_state_timer.start(5);
      //TODO
      break;
    case STATE_RECHARGING:
      std::cout<<"recharging"<<std::endl;
      set_action(m_attack == ATTACK_PINCH ? "charge-pinch" : "charge");
      m_state_timer.start(1);
      m_willo_to_spawn = m_attack == ATTACK_PINCH ? 9 : 3;
      break;
    case STATE_DEAD:
      std::cout<<"dead"<<std::endl;
      set_action("busted");
      run_dead_script();
      break;
    default:
      break;
  }
  m_state = new_state;
}

/*bool
GhostTree::is_color_deadly(Color color) const
{
  if (color == Color(0,0,0)) return false;
  Color my_color = glow_sprite->get_color();
  return ((my_color.red != color.red) || (my_color.green != color.green) || (my_color.blue != color.blue));
}*/

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

/*void
GhostTree::draw(DrawingContext& context)
{
  Boss::draw(context);

  context.push_transform();
  if (mystate == STATE_SUCKING) {
    context.set_alpha(0.5f + fmodf(g_game_time, 0.5f));
  } else {
    context.set_alpha(0.5f);
  }
  glow_sprite->draw(context.light(), get_pos(), m_layer);
  context.pop_transform();
}*/

bool
GhostTree::collides(MovingObject& other, const CollisionHit& ) const
{
  if (m_state != STATE_RECHARGING) return false;
  //if (dynamic_cast<Lantern*>(&other)) return true;
  if (dynamic_cast<Player*>(&other)) return true;
  return false;
}

HitResponse
GhostTree::collision(MovingObject& other, const CollisionHit& hit)
{
  if (m_state != STATE_RECHARGING) return ABORT_MOVE;
  return BadGuy::collision(other, hit);

  //TODO collision from above? subtract one life
  /*auto player = dynamic_cast<Player*>(&other);
  if (player) {
    player->kill(false);
  }

  Lantern* lantern = dynamic_cast<Lantern*>(&other);
  if (lantern) {
    suck_lantern = lantern;
    suck_lantern->grab(*this, suck_lantern->get_pos(), Direction::RIGHT);
    suck_lantern_color = lantern->get_color();
    mystate = STATE_SWALLOWING;
  }*/

  //return ABORT_MOVE;
}

bool
GhostTree::collision_squished(MovingObject& object)
{
  auto player = dynamic_cast<Player*>(&object);
  if (player) {
    player->bounce(*this);
  }

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


/*void
GhostTree::spawn_lantern()
{
  Sector::get().add<Lantern>(m_col.m_bbox.get_middle() + SUCK_TARGET_OFFSET);
}*/

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
