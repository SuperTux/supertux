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

#include "audio/sound_manager.hpp"

#include "audio/sound_source.hpp"

SoundManager::SoundManager() :
  m_sound_mgr()
{
}

void
SoundManager::play(const std::string& name, const Vector& pos, const float gain)
{
  // m_sound_mgr.play(name, pos, gain);
}

void
SoundManager::play(const std::string& name, const float gain)
{
  play(name, Vector(-1, -1), gain);
}

/** preloads a sound, so that you don't get a lag later when playing it */
void
SoundManager::preload(const std::string& name)
{
}

void
SoundManager::play_music(const std::string& filename, float fadetime)
{
}

void
SoundManager::play_music(const std::string& filename, bool fade)
{
}

void
SoundManager::pause_music(float fadetime)
{
}

void
SoundManager::resume_music(float fadetime)
{
}

void
SoundManager::stop_music()
{
}

void
SoundManager::stop_music(float fadetime)
{
}

std::string
SoundManager::get_current_music() const
{
  return {};
}

void
SoundManager::pause_sounds()
{
}
void
SoundManager::resume_sounds()
{
}
void
SoundManager::stop_sounds()
{
}

void
SoundManager::update()
{
#if 0
  if (m_music_source == nullptr)
    return;

  if (fadetime > 0) {
    if (m_music_source
       && m_music_source->get_fade_state() != StreamSoundSource::FadingResume) {
      m_music_source->set_fading(StreamSoundSource::FadingResume, fadetime);
      m_music_source->resume();
    }
  } else {
    m_music_source->resume();
  }
#endif
}

std::unique_ptr<SoundSource>
SoundManager::create_sound_source(const std::string& filename)
{
  return {};
}

void
SoundManager::set_listener_position(const Vector& position)
{
}

void
SoundManager::manage_source(std::unique_ptr<SoundSource> source)
{
}

void
SoundManager::enable_sound(bool sound_enabled)
{
}
void
SoundManager::enable_music(bool music_enabled)
{
}

void
SoundManager::set_music_volume(int volume)
{
}
void
SoundManager::set_sound_volume(int volume)
{
}

bool
SoundManager::is_audio_enabled() const
{
  return {};
}

/* EOF */
