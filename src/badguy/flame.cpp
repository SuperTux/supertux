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

#include "flame.hpp"
#include "log.hpp"

Flame::Flame(const lisp::Lisp& reader)
  : angle(0), radius(100), speed(2), source(0)
{
  reader.get("x", start_position.x);
  reader.get("y", start_position.y);
  reader.get("radius", radius);
  reader.get("speed", speed);
  bbox.set_pos(Vector(start_position.x + cos(angle) * radius,
                      start_position.y + sin(angle) * radius));
  sprite = sprite_manager->create("images/creatures/flame/flame.sprite");
  bbox.set_size(sprite->get_current_hitbox_width(), sprite->get_current_hitbox_height());
  countMe = false;
  layer = LAYER_FLOATINGOBJECTS;
}

Flame::Flame(const Flame& other)
  : BadGuy(other), angle(other.angle), radius(other.radius), speed(other.speed)
{
  if (sound_manager->is_sound_enabled()) {
    source.reset(sound_manager->create_sound_source("sounds/flame.wav"));
  }
}

void
Flame::write(lisp::Writer& writer)
{
  writer.start_list("flame");

  writer.write_float("x", start_position.x);
  writer.write_float("y", start_position.y);
  writer.write_float("radius", radius);
  writer.write_float("speed", speed);

  writer.end_list("flame");
}

void
Flame::active_update(float elapsed_time)
{
  angle = fmodf(angle + elapsed_time * speed, 2*M_PI);
  Vector newpos(start_position.x + cos(angle) * radius,
                start_position.y + sin(angle) * radius);
  movement = newpos - get_pos();

  if (sound_manager->is_sound_enabled())
    source->set_position(get_pos());
}

void
Flame::activate()
{
  set_group(COLGROUP_TOUCHABLE);

  if (!sound_manager->is_sound_enabled())
    return;

  source.reset(sound_manager->create_sound_source("sounds/flame.wav"));
  if(source.get() == NULL) {
    log_warning << "Couldn't start flame sound" << std::endl;
    return;
  }
  source->set_position(get_pos());
  source->set_looping(true);
  source->set_gain(2.0);
  source->set_reference_distance(32);
  source->play();
}

void
Flame::deactivate()
{
  source.release();
}

void
Flame::kill_fall()
{
}

IMPLEMENT_FACTORY(Flame, "flame")

