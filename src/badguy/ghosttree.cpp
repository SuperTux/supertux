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
#include "math/random_generator.hpp"
#include "object/lantern.hpp"
#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/sector.hpp"

static const size_t WILLOWISP_COUNT = 10;
static const float ROOT_TOP_OFFSET = 64;
static const float WILLOWISP_TOP_OFFSET = -64;
static const Vector SUCK_TARGET_OFFSET = Vector(-16,-16);
static const float SUCK_TARGET_SPREAD = 8;

GhostTree::GhostTree(const ReaderMapping& lisp) :
  BadGuy(lisp, "images/creatures/ghosttree/ghosttree.sprite", LAYER_OBJECTS - 10),
  mystate(STATE_IDLE),
  willowisp_timer(),
  willo_spawn_y(0),
  willo_radius(200),
  willo_speed(1.8f),
  willo_color(0),
  glow_sprite(SpriteManager::current()->create("images/creatures/ghosttree/ghosttree-glow.sprite")),
  colorchange_timer(),
  suck_timer(),
  root_timer(),
  treecolor(0),
  suck_lantern_color(),
  suck_lantern(0),
  willowisps()
{
  set_colgroup_active(COLGROUP_TOUCHABLE);
  SoundManager::current()->preload("sounds/tree_howling.ogg");
  SoundManager::current()->preload("sounds/tree_suck.ogg");
}

void
GhostTree::die()
{
  mystate = STATE_DYING;
  sprite->set_action("dying", 1);
  glow_sprite->set_action("dying", 1);

  for(const auto& willo : willowisps) {
    willo->vanish();
  }
  run_dead_script();
}

void
GhostTree::activate()
{
  willowisp_timer.start(1.0f, true);
  colorchange_timer.start(13, true);
  root_timer.start(5, true);
}

void
GhostTree::active_update(float /*elapsed_time*/)
{
  if (mystate == STATE_IDLE) {
    if(colorchange_timer.check()) {
      SoundManager::current()->play("sounds/tree_howling.ogg", get_pos());
      suck_timer.start(3);
      treecolor = (treecolor + 1) % 3;

      Color col;
      switch(treecolor) {
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

    if(suck_timer.check()) {
      Color col = glow_sprite->get_color();
      SoundManager::current()->play("sounds/tree_suck.ogg", get_pos());
      for(const auto& willo : willowisps) {
        if(willo->get_color() == col) {
          willo->start_sucking(
            bbox.get_middle() + SUCK_TARGET_OFFSET
            + Vector(gameRandom.randf(-SUCK_TARGET_SPREAD, SUCK_TARGET_SPREAD),
                     gameRandom.randf(-SUCK_TARGET_SPREAD, SUCK_TARGET_SPREAD)));
        }
      }
      mystate = STATE_SUCKING;
    }

    if(willowisp_timer.check()) {
      if(willowisps.size() < WILLOWISP_COUNT) {
        Vector pos = Vector(bbox.get_width() / 2, bbox.get_height() / 2 + willo_spawn_y + WILLOWISP_TOP_OFFSET);
        auto willowisp = std::make_shared<TreeWillOWisp>(this, pos, 200 + willo_radius, willo_speed);

        Sector::current()->add_object(willowisp);
        willowisps.push_back(willowisp);

        willo_spawn_y -= 40;
        if(willo_spawn_y < -160)
          willo_spawn_y = 0;

        willo_radius += 20;
        if(willo_radius > 120)
          willo_radius = 0;

        if(willo_speed == 1.8f) {
          willo_speed = 1.5f;
        } else {
          willo_speed = 1.8f;
        }

        do {
          willo_color = (willo_color + 1) % 3;
        } while(willo_color == treecolor);

        switch(willo_color) {
          case 0: willowisp->set_color(Color(1, 0, 0)); break;
          case 1: willowisp->set_color(Color(0, 1, 0)); break;
          case 2: willowisp->set_color(Color(0, 0, 1)); break;
          case 3: willowisp->set_color(Color(1, 1, 0)); break;
          case 4: willowisp->set_color(Color(1, 0, 1)); break;
          case 5: willowisp->set_color(Color(0, 1, 1)); break;
          default: assert(false);
        }
      }
    }

    if(root_timer.check()) {
      /* TODO indicate root with an animation */
      auto player = get_nearest_player();
      if (player) {
        auto root = std::make_shared<Root>(Vector(player->get_bbox().get_left(), bbox.get_bottom()+ROOT_TOP_OFFSET));
        Sector::current()->add_object(root);
      }
    }
  } else if (mystate == STATE_SWALLOWING) {
    if (suck_lantern) {
      // suck in lantern
      assert (suck_lantern);
      Vector pos = suck_lantern->get_pos();
      Vector delta = bbox.get_middle() + SUCK_TARGET_OFFSET - pos;
      Vector dir_ = delta.unit();
      if (delta.norm() < 1) {
        dir_ = delta;
        suck_lantern->ungrab(*this, RIGHT);
        suck_lantern->remove_me();
        suck_lantern = 0;
        sprite->set_action("swallow", 1);
      } else {
        pos += dir_;
        suck_lantern->grab(*this, pos, RIGHT);
      }
    } else {
      // wait until lantern is swallowed
      if (sprite->animation_done()) {
        if (is_color_deadly(suck_lantern_color)) {
          die();
        } else {
          sprite->set_action("default");
          mystate = STATE_IDLE;
          spawn_lantern();
        }
      }
    }
  }
}

bool
GhostTree::is_color_deadly(Color color) const
{
  if (color == Color(0,0,0)) return false;
  Color my_color = glow_sprite->get_color();
  return ((my_color.red != color.red) || (my_color.green != color.green) || (my_color.blue != color.blue));
}

void
GhostTree::willowisp_died(TreeWillOWisp *willowisp)
{
  if ((mystate == STATE_SUCKING) && (willowisp->was_sucked)) {
    mystate = STATE_IDLE;
  }
  willowisps.erase(std::find_if(willowisps.begin(), willowisps.end(),
                                [willowisp](const std::shared_ptr<TreeWillOWisp>& lhs)
                                {
                                  return lhs.get() == willowisp;
                                }));
}

void
GhostTree::draw(DrawingContext& context)
{
  BadGuy::draw(context);

  context.push_transform();
  if (mystate == STATE_SUCKING) {
    context.set_alpha(0.5f + fmodf(game_time, 0.5f));
  } else {
    context.set_alpha(0.5f);
  }
  glow_sprite->draw(context.light(), get_pos(), layer);
  context.pop_transform();
}

bool
GhostTree::collides(GameObject& other, const CollisionHit& ) const
{
  if (mystate != STATE_SUCKING) return false;
  if (dynamic_cast<Lantern*>(&other)) return true;
  if (dynamic_cast<Player*>(&other)) return true;
  return false;
}

HitResponse
GhostTree::collision(GameObject& other, const CollisionHit& )
{
  if(mystate != STATE_SUCKING) return ABORT_MOVE;

  auto player = dynamic_cast<Player*>(&other);
  if (player) {
    player->kill(false);
  }

  Lantern* lantern = dynamic_cast<Lantern*>(&other);
  if (lantern) {
    suck_lantern = lantern;
    suck_lantern->grab(*this, suck_lantern->get_pos(), RIGHT);
    suck_lantern_color = lantern->get_color();
    mystate = STATE_SWALLOWING;
  }

  return ABORT_MOVE;
}

void
GhostTree::spawn_lantern()
{
  auto lantern = std::make_shared<Lantern>(bbox.get_middle() + SUCK_TARGET_OFFSET);
  Sector::current()->add_object(lantern);
}

/* EOF */
