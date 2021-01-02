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

#include "object/ambient_sound.hpp"

#include <limits>

#include "audio/sound_manager.hpp"
#include "audio/sound_source.hpp"
#include "editor/editor.hpp"
#include "object/camera.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"
#include "util/writer.hpp"
#include "video/drawing_context.hpp"

AmbientSound::AmbientSound(const ReaderMapping& mapping) :
  MovingObject(mapping),
  ExposedObject<AmbientSound, scripting::AmbientSound>(this),
  sample(),
  sound_source(),
  latency(),
  distance_factor(),
  distance_bias(),
  silence_distance(),
  maximumvolume(),
  targetvolume(),
  currentvolume(0)
{
  m_col.m_group = COLGROUP_DISABLED;

  float w, h;
  mapping.get("x", m_col.m_bbox.get_left(), 0.0f);
  mapping.get("y", m_col.m_bbox.get_top(), 0.0f);
  mapping.get("width" , w, 32.0f);
  mapping.get("height", h, 32.0f);
  m_col.m_bbox.set_size(w, h);

  mapping.get("distance_factor",distance_factor, 0.0f);
  mapping.get("distance_bias"  ,distance_bias  , 0.0f);
  mapping.get("sample"         ,sample         , "");
  mapping.get("volume"         ,maximumvolume  , 1.0f);

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

  mapping.get("silence_distance",silence_distance);

  if (!Editor::is_active()) {
    sound_source.reset(); // not playing at the beginning
    SoundManager::current()->preload(sample);
  }
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
  currentvolume()
{
  m_col.m_group = COLGROUP_DISABLED;

  m_col.m_bbox.set_pos(pos);
  m_col.m_bbox.set_size(32, 32);

  // set default silence_distance

  if (distance_factor == 0)
    silence_distance = std::numeric_limits<float>::max();
  else
    silence_distance = 1/distance_factor;

  if (!Editor::is_active()) {
    sound_source.reset(); // not playing at the beginning
    SoundManager::current()->preload(sample);
  }
}

AmbientSound::~AmbientSound()
{
  stop_playing();
}

ObjectSettings
AmbientSound::get_settings()
{
  ObjectSettings result = MovingObject::get_settings();

  result.add_sound(_("Sound"), &sample, "sample");
  result.add_float(_("Distance factor"), &distance_factor, "distance_factor");
  result.add_float(_("Distance bias"), &distance_bias, "distance_bias");
  result.add_float(_("Volume"), &maximumvolume, "volume");

  result.reorder({"sample", "distance_factor", "distance_bias", "volume", "region", "name", "x", "y", "width", "height"});

  return result;
}

void
AmbientSound::after_editor_set()
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
    if (!sound_source)
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
AmbientSound::update(float dt_sec)
{
  if (latency-- <= 0) {
    float px,py;
    float rx,ry;

    // Camera position
    px=Sector::get().get_camera().get_center().x;
    py=Sector::get().get_camera().get_center().y;

    // Relate to which point in the area
    rx=px<m_col.m_bbox.get_left()?m_col.m_bbox.get_left():
      (px<m_col.m_bbox.get_right()?px:m_col.m_bbox.get_right());
    ry=py<m_col.m_bbox.get_top()?m_col.m_bbox.get_top():
      (py<m_col.m_bbox.get_bottom()?py:m_col.m_bbox.get_bottom());

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
    currentvolume *= powf(rise, dt_sec * 10.0f);
    currentvolume += 1e-6f; // volume is at least 1e-6 (0 would never rise)

    if (sound_source != nullptr) {

      // set the volume
      sound_source->set_gain(currentvolume*maximumvolume);

      if (sqrdistance>=silence_distance && currentvolume < 1e-3f)
        stop_playing();
      latency=0;
    } else {
      if (sqrdistance<silence_distance) {
        start_playing();
        latency=0;
      }
      else // set a reasonable latency
        latency = static_cast<int>(0.001f / distance_factor);
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
  m_col.m_bbox.set_pos(Vector(x, y));
}

float
AmbientSound::get_pos_x() const
{
  return m_col.m_bbox.get_left();
}

float
AmbientSound::get_pos_y() const
{
  return m_col.m_bbox.get_top();
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
    context.color().draw_filled_rect(m_col.m_bbox, Color(0.0f, 0.0f, 1.0f, 0.6f),
                                     0.0f, LAYER_OBJECTS);
  }
}

BEGIN_BACKUP(AmbientSound, MovingObject);
  SAVE_PRIMITIVE(sample);
  bool playing = sound_source->playing();
  SAVE_PRIMITIVE(playing);
  SAVE_PRIMITIVE(latency);

  SAVE_PRIMITIVE(distance_factor);
  SAVE_PRIMITIVE(distance_bias);
  SAVE_PRIMITIVE(silence_distance);

  SAVE_PRIMITIVE(maximumvolume);
  SAVE_PRIMITIVE(targetvolume);
  SAVE_PRIMITIVE(currentvolume);
END_BACKUP(AmbientSound);

BEGIN_RESTORE(AmbientSound, MovingObject);
  LOAD_PRIMITIVE(sample);

  bool playing = false;
  LOAD_PRIMITIVE(playing);
  if (playing)
    start_playing();

  LOAD_PRIMITIVE(latency);

  LOAD_PRIMITIVE(distance_factor);
  LOAD_PRIMITIVE(distance_bias);
  LOAD_PRIMITIVE(silence_distance);

  LOAD_PRIMITIVE(maximumvolume);
  LOAD_PRIMITIVE(targetvolume);
  LOAD_PRIMITIVE(currentvolume);
END_RESTORE(AmbientSound);

/* EOF */
