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

#include <config.h>

#include <math.h>
#include <stdexcept>
#include <iostream>
#include <limits>

#include "object/ambient_sound.hpp"
#include "supertux/object_factory.hpp"
#include "lisp/lisp.hpp"
#include "supertux/sector.hpp"
#include "audio/sound_manager.hpp"
#include "audio/sound_source.hpp"
#include "util/log.hpp"
#include "scripting/squirrel_util.hpp"
#include "object/camera.hpp"

AmbientSound::AmbientSound(const lisp::Lisp& lisp)
{
  name="";
  position.x = 0;
  position.y = 0;

  dimension.x = 0;
  dimension.y = 0;

  distance_factor = 0;
  distance_bias = 0;
  maximumvolume = 1;
  sample = "";
  currentvolume = 0;

  if (!(lisp.get("x", position.x)&&lisp.get("y", position.y))) {
    log_warning << "No Position in ambient_sound" << std::endl;
  }

  lisp.get("name" , name);
  lisp.get("width" , dimension.x);
  lisp.get("height", dimension.y);

  lisp.get("distance_factor",distance_factor);
  lisp.get("distance_bias"  ,distance_bias  );
  lisp.get("sample"         ,sample         );
  lisp.get("volume"         ,maximumvolume  );

  // set dimension to zero if smaller than 64, which is default size in flexlay

  if ((dimension.x <= 64) || (dimension.y <= 64)) {
    dimension.x = 0;
    dimension.y = 0;
  }

  // square all distances (saves us a sqrt later)

  distance_bias*=distance_bias;
  distance_factor*=distance_factor;

  // set default silence_distance

  if (distance_factor == 0)
          silence_distance = std::numeric_limits<float>::max();
  else
    silence_distance = 1/distance_factor;

  lisp.get("silence_distance",silence_distance);

  sound_source = 0; // not playing at the beginning
  sound_manager->preload(sample);
  latency=0;
}

AmbientSound::AmbientSound(Vector pos, float factor, float bias, float vol, std::string file)
{
  position.x=pos.x;
  position.y=pos.y;

  dimension.x=0;
  dimension.y=0;

  distance_factor=factor*factor;
  distance_bias=bias*bias;
  maximumvolume=vol;
  sample=file;

  // set default silence_distance

  if (distance_factor == 0)
          silence_distance = std::numeric_limits<float>::max();
  else
    silence_distance = 1/distance_factor;

  sound_source = 0; // not playing at the beginning
  sound_manager->preload(sample);
  latency=0;
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
    currentvolume=targetvolume=1e-20f;
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

    if (!Sector::current() || !Sector::current()->camera) return;
    // Camera position
    px=Sector::current()->camera->get_center().x;
    py=Sector::current()->camera->get_center().y;

    // Relate to which point in the area
    rx=px<position.x?position.x:
      (px<position.x+dimension.x?px:position.x+dimension.x);
    ry=py<position.y?position.y:
      (py<position.y+dimension.y?py:position.y+dimension.y);

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
    currentvolume += 1e-6f; // volume is at least 1e-6 (0 would never rise)

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
  Scripting::AmbientSound* interface = static_cast<Scripting::AmbientSound*> (this);
  expose_object(vm, table_idx, interface, name, false);
}

void
AmbientSound::unexpose(HSQUIRRELVM vm, SQInteger table_idx)
{
  Scripting::unexpose_object(vm, table_idx, name);
}

void
AmbientSound::set_pos(float x, float y)
{
  position.x = x;
  position.y = y;
}

float
AmbientSound::get_pos_x() const
{
  return position.x;
}

float
AmbientSound::get_pos_y() const
{
  return position.y;
}

IMPLEMENT_FACTORY(AmbientSound, "ambient_sound");
