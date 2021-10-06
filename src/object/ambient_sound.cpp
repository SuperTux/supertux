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
#include "video/drawing_context.hpp"
#include "object/player.hpp"

AmbientSound::AmbientSound(const ReaderMapping& mapping) :
  MovingObject(mapping),
  ExposedObject<AmbientSound, scripting::AmbientSound>(this),
  sample(),
  sound_source(),
  m_effect_distance(),
  targetvolume(1)
{
  m_col.m_group = COLGROUP_DISABLED;

  float w, h;
  mapping.get("x", m_col.m_bbox.get_left(), 0.0f);
  mapping.get("y", m_col.m_bbox.get_top(), 0.0f);
  mapping.get("width" , w, 32.0f);
  mapping.get("height", h, 32.0f);
  m_col.m_bbox.set_size(w, h);

  mapping.get("effect_distance",m_effect_distance, 0.0f);
  mapping.get("sample"         ,sample           , "");
  mapping.get("volume"         ,targetvolume     , 1.0f);

  if (!Editor::is_active()) {
    sound_source.reset(); // not playing at the beginning
    SoundManager::current()->preload(sample);
  }
}

AmbientSound::AmbientSound(const Vector& pos, float distance, float vol, const std::string& file) :
  ExposedObject<AmbientSound, scripting::AmbientSound>(this),
  sample(file),
  sound_source(),
  m_effect_distance(distance),
  targetvolume(vol)
{
  m_col.m_group = COLGROUP_DISABLED;

  m_col.m_bbox.set_pos(pos);
  m_col.m_bbox.set_size(32, 32);

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
  result.add_float(_("Effect distance"), &m_effect_distance, "effect_distance");
  result.add_float(_("Volume"), &targetvolume, "volume");

  result.reorder({"sample", "effect_distance", "volume", "region", "name", "x", "y", "width", "height"});

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
    sound_source->set_relative(true);
    sound_source->set_looping(true);
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
  // check if player is within 100% volume area
  if (m_col.get_bbox().contains(Sector::get().get_player().get_bbox()))
  {
    if (sound_source == nullptr)
      start_playing();
    sound_source->set_gain(targetvolume);
  }
  else
  {
    Vector player_pos = Sector::get().get_player().get_pos();
    float dist = 0;
    if (player_pos.y >= get_pos().y && player_pos.y <= get_pos().y + get_bbox().get_height()) // Player is either right or left to ambient sound
    {
      if (player_pos.x <= get_pos().x)
        dist = get_pos().x - player_pos.x;
      else
        dist = player_pos.x - get_pos().x - get_bbox().get_width();
    }
    else if (player_pos.x >= get_pos().x && player_pos.x <= get_pos().x + get_bbox().get_width()) // Player is either above or under ambient sound
    {
      if (player_pos.y <= get_pos().y)
        dist = get_pos().y - player_pos.y;
      else
        dist = player_pos.y - get_pos().y - get_bbox().get_height();
    }
    else // Player is in one of the corners
    {
      if (player_pos.x <= get_pos().x && player_pos.y <= get_pos().y)
        dist = Sector::get().get_player().get_bbox().distance(m_col.get_bbox(), AnchorPoint::ANCHOR_TOP_LEFT);
      else if (player_pos.x >= get_pos().x + get_bbox().get_width() && player_pos.y <= get_pos().y)
        dist = Sector::get().get_player().get_bbox().distance(m_col.get_bbox(), AnchorPoint::ANCHOR_TOP_RIGHT);
      else if (player_pos.x <= get_pos().x && player_pos.y >= get_pos().y + get_bbox().get_height())
        dist = Sector::get().get_player().get_bbox().distance(m_col.get_bbox(), AnchorPoint::ANCHOR_BOTTOM_LEFT);
      else if (player_pos.x >= get_pos().x + get_bbox().get_width() && player_pos.y >= get_pos().y + get_bbox().get_height())
        dist = Sector::get().get_player().get_bbox().distance(m_col.get_bbox(), AnchorPoint::ANCHOR_BOTTOM_RIGHT);
    }
    
    if (dist < m_effect_distance || m_effect_distance == -1)
    {
      if (sound_source==nullptr)
        start_playing();
      else if (m_effect_distance == -1)
        sound_source->set_gain(targetvolume);
      else
        sound_source->set_gain((1.0f - (dist / m_effect_distance)) * targetvolume);   
    }
    if (dist >= m_effect_distance && m_effect_distance != -1)
      stop_playing();
  }
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

/* EOF */
