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

#ifndef HEADER_SUPERTUX_AUDIO_SOUND_MANAGER_HPP
#define HEADER_SUPERTUX_AUDIO_SOUND_MANAGER_HPP

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <al.h>
#include <alc.h>

#include <wstsound/sound_manager.hpp>

#include "audio/fwd.hpp"
#include "math/vector.hpp"
#include "util/currenton.hpp"

class SoundManager final : public Currenton<SoundManager>
{
public:
  SoundManager();

  /** Convenience functions to simply play a sound at a given position. */
  void play(const std::string& name, const Vector& pos = Vector(-1, -1),
            const float gain = 0.5f);
  void play(const std::string& name, const float gain);

  /** preloads a sound, so that you don't get a lag later when playing it */
  void preload(const std::string& name);

  void play_music(const std::string& filename, float fadetime);
  void play_music(const std::string& filename, bool fade = false);

  void pause_music(float fadetime = 0);
  void resume_music(float fadetime = 0);

  void stop_music();
  void stop_music(float);

  std::string get_current_music() const;

  void pause_sounds();
  void resume_sounds();
  void stop_sounds();

  void update();

  std::unique_ptr<SoundSource> create_sound_source(const std::string& filename);

  void set_listener_position(const Vector& position);

  void manage_source(std::unique_ptr<SoundSource> source);

  void enable_sound(bool sound_enabled);
  void enable_music(bool music_enabled);

  void set_music_volume(int volume);
  void set_sound_volume(int volume);

  bool is_audio_enabled() const;

private:
  wstsound::SoundManager m_sound_mgr;

private:
  SoundManager(const SoundManager&) = delete;
  SoundManager& operator=(const SoundManager&) = delete;
};

#endif

/* EOF */
