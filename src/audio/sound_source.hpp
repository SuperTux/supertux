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

#ifndef HEADER_SUPERTUX_AUDIO_SOUND_SOURCE_HPP
#define HEADER_SUPERTUX_AUDIO_SOUND_SOURCE_HPP

#include <wstsound/sound_source.hpp>

#include "math/fwd.hpp"

class SoundSource
{
public:
  SoundSource(wstsound::SoundSourcePtr source);

  void play();
  void stop();
  bool playing() const;
  void set_pitch(float pitch);
  void set_position(const Vector& pos);
  void set_looping(bool looping);
  void set_gain(float gain);
  void set_reference_distance(float distance);

private:
  wstsound::SoundSourcePtr m_source;

private:
  SoundSource(const SoundSource&) = delete;
  SoundSource& operator=(const SoundSource&) = delete;
};

#endif

/* EOF */
