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

#include <limits>
#include <math.h>

#include "audio/sound_manager.hpp"
#include "audio/sound_source.hpp"
#include "editor/editor.hpp"
#include "object/ambient_sound.hpp"
#include "object/camera.hpp"
#include "scripting/squirrel_util.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"
#include "video/drawing_context.hpp"

AmbientSound::AmbientSound(const ReaderMapping& lisp) :
  ExposedObject<AmbientSound, scripting::AmbientSound>(this),
  sample(),
  sound_source(),
  latency(),
  distance_factor(),
  distance_bias(),
  silence_distance(),
  maximumvolume(),
  targetvolume(),
  currentvolume(0),
  volume_ptr(),
  new_size()
{
  group = COLGROUP_DISABLED;

  float w, h;
  lisp.get("name" , name, "");
  lisp.get("x", bbox.p1.x, 0);
  lisp.get("y", bbox.p1.y, 0);
  lisp.get("width" , w, 32);
  lisp.get("height", h, 32);
  bbox.set_size(w, h);

  lisp.get("distance_factor",distance_factor, 0);
  lisp.get("distance_bias"  ,distance_bias  , 0);
  lisp.get("sample"         ,sample         , "");
  lisp.get("volume"         ,maximumvolume  , 1);

  // square all distances (saves us a sqrt later)

  if (!Editor::is_active()) {
    distance_bias*=distance_bias;
    distance_factor*=distance_factor;
  }

  // set default silence_distance

  if (distance_factor == 0)
    silence_distance = std::numeric_limits<float>::max();
  else
    silence_distance = 1/distance_factor;

  lisp.get("silence_distance",silence_distance);

  sound_source.reset(); // not playing at the beginning
  SoundManager::current()->preload(sample);
  latency=0;
}

AmbientSound::AmbientSound(const Vector& pos, float factor, float bias, float vol, const std::string& file) :
  ExposedObject<AmbientSound, scripting::AmbientSound>(this),
  sample(file),
  sound_source(),
  latency(0),
  distance_factor(factor * factor),
  distance_bias(bias * bias),
  silence_distance(),
  maximumvolume(vol),
  targetvolume(),
  currentvolume(),
  volume_ptr(),
  new_size()
{
  group = COLGROUP_DISABLED;

  bbox.set_pos(pos);
  bbox.set_size(0, 0);

  // set default silence_distance

  if (distance_factor == 0)
    silence_distance = std::numeric_limits<float>::max();
  else
    silence_distance = 1/distance_factor;

  sound_source.reset(); // not playing at the beginning
  SoundManager::current()->preload(sample);
}

AmbientSound::~AmbientSound()
{
  stop_playing();
}

ObjectSettings
AmbientSound::get_settings() {
  new_size.x = bbox.get_width();
  new_size.y = bbox.get_height();
  ObjectSettings result = MovingObject::get_settings();

  ObjectOption smp(MN_FILE, _("Sound"), &sample, "sample");
  smp.select.push_back(".wav");
  smp.select.push_back(".ogg");
  result.options.push_back(smp);
  result.options.push_back( ObjectOption(MN_NUMFIELD, _("Width"), &new_size.x, "width"));
  result.options.push_back( ObjectOption(MN_NUMFIELD, _("Height"), &new_size.y, "height"));
  result.options.push_back( ObjectOption(MN_NUMFIELD, _("Distance factor"), &distance_factor, "distance_factor"));
  result.options.push_back( ObjectOption(MN_NUMFIELD, _("Distance bias"), &distance_bias, "distance_bias"));
  result.options.push_back( ObjectOption(MN_NUMFIELD, _("Volume"), &maximumvolume, "volume"));
  return result;
}

void
AmbientSound::after_editor_set() {
  bbox.set_size(new_size.x, new_size.y);
}

void
AmbientSound::hit(Player& )
{
}

void
AmbientSound::stop_playing()
{
  sound_source.reset();
}

void
AmbientSound::start_playing()
{
  if (Editor::is_active()) return;

  try {
    sound_source = SoundManager::current()->create_sound_source(sample);
    if(!sound_source)
      throw std::runtime_error("file not found");

    sound_source->set_gain(0);
    sound_source->set_looping(true);
    currentvolume=targetvolume=1e-20f;
    sound_source->play();
  } catch(std::exception& e) {
    log_warning << "Couldn't play '" << sample << "': " << e.what() << "" << std::endl;
    sound_source.reset();
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

#ifndef SCRIPTING_API
void
AmbientSound::set_pos(const Vector& pos)
{
  MovingObject::set_pos(pos);
}
#endif

void
AmbientSound::set_pos(float x, float y)
{
  bbox.set_pos(Vector(x, y));
}

float
AmbientSound::get_pos_x() const
{
  return bbox.p1.x;
}

float
AmbientSound::get_pos_y() const
{
  return bbox.p1.y;
}

HitResponse
AmbientSound::collision(GameObject& other, const CollisionHit& hit_)
{
  return ABORT_MOVE;
}

void
AmbientSound::draw(DrawingContext& context)
{
  if (Editor::is_active()) {
    context.draw_filled_rect(bbox, Color(0.0f, 0.0f, 1.0f, 0.6f),
                             0.0f, LAYER_OBJECTS);
  }
}

/* EOF */
