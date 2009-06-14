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
#include "lisp/writer.hpp"
#include "lisp/lisp.hpp"
#include "object_factory.hpp"
#include "audio/sound_manager.hpp"

#include <math.h>

static const std::string SOUNDFILE = "sounds/flame.wav";

Flame::Flame(const lisp::Lisp& reader)
  : BadGuy(reader, "images/creatures/flame/flame.sprite", LAYER_FLOATINGOBJECTS), angle(0), radius(100), speed(2)
{
  reader.get("radius", radius);
  reader.get("speed", speed);
  bbox.set_pos(Vector(start_position.x + cos(angle) * radius,
                      start_position.y + sin(angle) * radius));
  countMe = false;
  sound_manager->preload(SOUNDFILE);

  set_colgroup_active(COLGROUP_TOUCHABLE);
}

void
Flame::write(lisp::Writer& writer)
{
  writer.start_list("flame");

  writer.write("x", start_position.x);
  writer.write("y", start_position.y);
  writer.write("radius", radius);
  writer.write("speed", speed);

  writer.end_list("flame");
}

void
Flame::active_update(float elapsed_time)
{
  angle = fmodf(angle + elapsed_time * speed, (float) (2*M_PI));
  Vector newpos(start_position.x + cos(angle) * radius,
                start_position.y + sin(angle) * radius);
  movement = newpos - get_pos();

  sound_source->set_position(get_pos());
}

void
Flame::activate()
{
  sound_source.reset(sound_manager->create_sound_source(SOUNDFILE));
  sound_source->set_position(get_pos());
  sound_source->set_looping(true);
  sound_source->set_gain(2.0);
  sound_source->set_reference_distance(32);
  sound_source->play();
}

void
Flame::deactivate()
{
  sound_source.reset();
}

void
Flame::kill_fall()
{
}

IMPLEMENT_FACTORY(Flame, "flame")
