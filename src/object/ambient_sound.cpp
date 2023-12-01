//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//                2023 mrkubax10 <mrkubax10@onet.pl>
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
#include "util/reader_mapping.hpp"

AmbientSound::AmbientSound(const ReaderMapping& mapping) :
  GameObject(mapping),
  ExposedObject<AmbientSound, scripting::AmbientSound>(this),
  m_sample(),
  m_sound_source(),
  m_volume(),
  m_play_interval(),
  m_delay(0.0f),
  m_playing(false),
  m_scheduled_for_removal(false)
{
  mapping.get("sample", m_sample, "");
  mapping.get("volume", m_volume, 1.0f);
  mapping.get("play-interval", m_play_interval, 0.0f);

  prepare_sound_source();
}

AmbientSound::AmbientSound(float vol, float play_interval, const std::string& file) :
  ExposedObject<AmbientSound, scripting::AmbientSound>(this),
  m_sample(file),
  m_sound_source(),
  m_volume(vol),
  m_play_interval(play_interval),
  m_delay(0.0f),
  m_playing(false),
  m_scheduled_for_removal(false)
{
  prepare_sound_source();
}

AmbientSound::~AmbientSound()
{
  stop_looping_sounds();
}

void
AmbientSound::update(float dt_sec)
{
  if (m_scheduled_for_removal)
  {
    remove_me();
    return;
  }

  if (!m_sound_source->playing())
  {
    if (m_playing && m_delay>=m_play_interval)
    {
      m_sound_source->play();
      m_delay = 0.0f;
    }
    m_delay+=dt_sec;
  }
}

ObjectSettings
AmbientSound::get_settings()
{
  ObjectSettings result = GameObject::get_settings();

  result.add_sound(_("Sound"), &m_sample, "sample");
  result.add_float(_("Volume"), &m_volume, "volume");
  result.add_float(_("Play interval"), &m_play_interval, "play-interval");

  result.reorder({"sample", "volume", "play-interval", "name"});

  return result;
}

void
AmbientSound::stop_looping_sounds()
{
  if (m_sound_source)
  {
    m_sound_source->stop(false);
    m_playing = false;
  }
}

void
AmbientSound::play_looping_sounds()
{
  if (!Editor::is_active() && m_sound_source)
  {
    m_playing = true;
    m_delay = 0.0f;
  }
}

void
AmbientSound::prepare_sound_source()
{
  try
  {
    m_sound_source = SoundManager::current()->create_sound_source(m_sample);
    if (!m_sound_source)
      throw std::runtime_error("file not found");
    m_sound_source->set_gain(m_volume);
    m_sound_source->set_relative(true);

    if (!Editor::is_active())
    {
      m_playing = true;
      m_delay = 0.0f;
      m_sound_source->play();
    }
  }
  catch(const std::exception& e)
  {
    log_warning << "Couldn't play '" << m_sample << "': " << e.what() << "" << std::endl;
    m_sound_source.reset();
    m_scheduled_for_removal = true;
  }
}

/* EOF */
