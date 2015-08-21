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

#include "badguy/poisonivy.hpp"
#include "math/random_generator.hpp"
#include "object/sprite_particle.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"
#include "util/gettext.hpp"

#include <math.h>

PoisonIvy::PoisonIvy(const ReaderMapping& reader)
  : WalkingBadguy(reader, "images/creatures/poison_ivy/poison_ivy.sprite", "left", "right")
{
  walk_speed = 80;
}

PoisonIvy::PoisonIvy(const Vector& pos, Direction d)
  : WalkingBadguy(pos, d, "images/creatures/poison_ivy/poison_ivy.sprite", "left", "right")
{
  walk_speed = 80;
}

bool
PoisonIvy::is_freezable() const
{
  return true;
}

bool
PoisonIvy::collision_squished(GameObject& object)
{
  sprite->set_action(dir == LEFT ? "squished-left" : "squished-right");
  // spawn some particles
  // TODO: provide convenience function in MovingSprite or MovingObject?
  for (int i = 0; i < 3; i++) {
    Vector ppos = bbox.get_middle();
    float angle = graphicsRandom.randf(-M_PI_2, M_PI_2);
    float velocity = graphicsRandom.randf(350, 400);
    float vx = sin(angle)*velocity;
    float vy = -cos(angle)*velocity;
    Vector pspeed = Vector(vx, vy);
    Vector paccel = Vector(0, Sector::current()->get_gravity()*10);
    Sector::current()->add_object(std::make_shared<SpriteParticle>("images/objects/particles/poisonivy.sprite",
                                                                   "default",
                                                                   ppos, ANCHOR_MIDDLE,
                                                                   pspeed, paccel,
                                                                   LAYER_OBJECTS-1));
  }
  kill_squished(object);
  return true;
}


ObjectSettings
PoisonIvy::get_settings() {
  ObjectSettings result(_("Poisonous ivy"));
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Name"), &name));
  result.options.push_back( dir_option(&dir) );
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Death script"), &dead_script));
  return result;
}

/* EOF */
