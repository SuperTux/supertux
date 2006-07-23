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

#include <math.h>
#include <stdexcept>
#include <iostream>

#include "ambient_sound.hpp"
#include "object_factory.hpp"
#include "lisp/lisp.hpp"
#include "sector.hpp"
#include "audio/sound_manager.hpp"
#include "audio/sound_source.hpp"
#include "log.hpp"
#include "scripting/squirrel_util.hpp"

AmbientSound::AmbientSound(const lisp::Lisp& lisp) :
  MovingObject(lisp), sample(""), sound_source(0), latency(0),
  distance_factor(0), distance_bias(0), maximumvolume(1), currentvolume(0)
{
  lisp.get("distance_factor",distance_factor);
  lisp.get("distance_bias"  ,distance_bias  );
  lisp.get("sample"         ,sample         );
  lisp.get("volume"         ,maximumvolume  );

  // set dimension to zero if smaller than 64, which is default size in flexlay

  if ((get_width() <= 64) || (get_height() <= 64)) {
    set_size(0, 0);
  }

  // square all distances (saves us a sqrt later)

  distance_bias*=distance_bias;
  distance_factor*=distance_factor;

  // set default silence_distance

  if (distance_factor == 0)
    silence_distance = 10e99;
  else
    silence_distance = 1/distance_factor;

  lisp.get("silence_distance",silence_distance);
}

AmbientSound::AmbientSound(Vector pos, float factor, float bias, float vol, std::string file) :
  sample(file), sound_source(0), latency(0), distance_factor(factor*factor),
  distance_bias(bias*bias), maximumvolume(vol), currentvolume(0)
{
  bbox.p1 = pos;
  bbox.p2 = pos;

  // set default silence_distance

  if (distance_factor == 0)
    silence_distance = 10e99;
  else
    silence_distance = 1/distance_factor;
}

AmbientSound::~AmbientSound() {
  stop_playing();
}

void
AmbientSound::hit(Player& )
{
}

void
AmbientSound::stop_playing() {
  delete sound_source;
  sound_source = 0;
}

void
AmbientSound::start_playing()
{
  try {
    sound_source = sound_manager->create_sound_source(sample);
    if(!sound_source)
      throw std::runtime_error("file not found");

    sound_source->set_gain(0);
    sound_source->set_looping(true);
    currentvolume=targetvolume=1e-20;
    sound_source->play();
  } catch(std::exception& e) {
    log_warning << "Couldn't play '" << sample << "': " << e.what() << "" << std::endl;
    delete sound_source;
    sound_source = 0;
    remove_me();
  }
}

void
AmbientSound::update(float deltat)
{
  if (latency-- <= 0) {
    float px,py;
    float rx,ry;

    // Player position
    px=Sector::current()->player->get_pos().x;
    py=Sector::current()->player->get_pos().y;

    // Relate to which point in the area
    rx=px<bbox.p1.x?bbox.p1.x:
      (px<bbox.p2.x?px:bbox.p2.x);
    ry=py<bbox.p1.y?bbox.p1.y:
      (py<bbox.p2.y?py:bbox.p2.y);

    // calculate square of distance
    float sqrdistance=(px-rx)*(px-rx)+(py-ry)*(py-ry);
    sqrdistance-=distance_bias;

    // inside the bias: full volume (distance 0)
    if (sqrdistance<0)
      sqrdistance=0;

    // calculate target volume - will never become 0
    targetvolume=1/(1+sqrdistance*distance_factor);
    float rise=targetvolume/currentvolume;

    // rise/fall half life?
    currentvolume*=pow(rise,deltat*10);
    currentvolume += 1e-6; // volume is at least 1e-6 (0 would never rise)

    if (sound_source != 0) {

      // set the volume
      sound_source->set_gain(currentvolume*maximumvolume);

      if (sqrdistance>=silence_distance && currentvolume<1e-3)
      stop_playing();
      latency=0;
    } else {
      if (sqrdistance<silence_distance) {
      start_playing();
      latency=0;
      }
      else // set a reasonable latency
      latency=(int)(0.001/distance_factor);
      //(int)(10*((sqrdistance-silence_distance)/silence_distance));
    }
  }

  // heuristically measured "good" latency maximum

  //  if (latency>0.001/distance_factor)
  // latency=
}

void
AmbientSound::draw(DrawingContext &)
{
}

void
AmbientSound::expose(HSQUIRRELVM vm, SQInteger table_idx)
{
  if(name.empty()) return;
  expose_object(vm, table_idx, dynamic_cast<Scripting::AmbientSound *>(this), name, false);
}

void
AmbientSound::unexpose(HSQUIRRELVM vm, SQInteger table_idx)
{
  if(name.empty()) return;
  Scripting::unexpose_object(vm, table_idx, name);
}

IMPLEMENT_FACTORY(AmbientSound, "ambient_sound");
