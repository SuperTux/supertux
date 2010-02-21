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

#include "badguy/mrtree.hpp"

#include "audio/sound_manager.hpp"
#include "badguy/poisonivy.hpp"
#include "badguy/stumpy.hpp"
#include "math/random_generator.hpp"
#include "object/player.hpp"
#include "object/sprite_particle.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"

#include <math.h>

static const float TREE_SPEED = 100;

static const float POISONIVY_WIDTH = 32;
static const float POISONIVY_HEIGHT = 32;
static const float POISONIVY_Y_OFFSET = 24;

MrTree::MrTree(const Reader& reader)
  : WalkingBadguy(reader, "images/creatures/mr_tree/mr_tree.sprite","left","right")
{
  walk_speed = TREE_SPEED;
  max_drop_height = 16;
  sound_manager->preload("sounds/mr_tree.ogg");
}

bool
MrTree::collision_squished(GameObject& object)
{
  // replace with Stumpy
  Vector stumpy_pos = get_pos();
  stumpy_pos.x += 20;
  stumpy_pos.y += 25;
  Stumpy* stumpy = new Stumpy(stumpy_pos, dir);
  remove_me();
  Sector::current()->add_object(stumpy);

  // give Feedback
  sound_manager->play("sounds/mr_tree.ogg", get_pos());
  Player* player = dynamic_cast<Player*>(&object);
  if (player) player->bounce(*this);

  // spawn some particles
  // TODO: provide convenience function in MovingSprite or MovingObject?
  for (int px = (int)stumpy->get_bbox().p1.x; px < (int)stumpy->get_bbox().p2.x; px+=10) {
    Vector ppos = Vector(px, stumpy->get_bbox().p1.y-5);
    float angle = graphicsRandom.randf(-M_PI_2, M_PI_2);
    float velocity = graphicsRandom.randf(45, 90);
    float vx = sin(angle)*velocity;
    float vy = -cos(angle)*velocity;
    Vector pspeed = Vector(vx, vy);
    Vector paccel = Vector(0, 100);
    Sector::current()->add_object(new SpriteParticle("images/objects/particles/leaf.sprite", "default", ppos, ANCHOR_MIDDLE, pspeed, paccel, LAYER_OBJECTS-1));
  }

  // spawn PoisonIvy
  Vector leaf1_pos(stumpy_pos.x - POISONIVY_WIDTH - 1, stumpy_pos.y - POISONIVY_Y_OFFSET);
  Rectf leaf1_bbox(leaf1_pos.x, leaf1_pos.y, leaf1_pos.x + POISONIVY_WIDTH, leaf1_pos.y + POISONIVY_HEIGHT);
  if (Sector::current()->is_free_of_movingstatics(leaf1_bbox, this)) {
    PoisonIvy* leaf1 = new PoisonIvy(leaf1_bbox.p1, LEFT);
    leaf1 = leaf1;
    leaf1->countMe = false;
    Sector::current()->add_object(leaf1);
  }

  // spawn PoisonIvy
  Vector leaf2_pos(stumpy_pos.x + sprite->get_current_hitbox_width() + 1, stumpy_pos.y - POISONIVY_Y_OFFSET);
  Rectf leaf2_bbox(leaf2_pos.x, leaf2_pos.y, leaf2_pos.x + POISONIVY_WIDTH, leaf2_pos.y + POISONIVY_HEIGHT);
  if (Sector::current()->is_free_of_movingstatics(leaf2_bbox, this)) {
    PoisonIvy* leaf2 = new PoisonIvy(leaf2_bbox.p1, RIGHT);
    leaf2 = leaf2;
    leaf2->countMe = false;
    Sector::current()->add_object(leaf2);
  }

  return true;
}

/* EOF */
