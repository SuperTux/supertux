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

#ifndef HEADER_SUPERTUX_AUDIO_OPENAL_SOUND_SOURCE_HPP
#define HEADER_SUPERTUX_AUDIO_OPENAL_SOUND_SOURCE_HPP

#include <al.h>

#include "audio/sound_source.hpp"

class OpenALSoundSource : public SoundSource
{
public:
  OpenALSoundSource();
  virtual ~OpenALSoundSource();

  virtual void play();
  virtual void stop();
  virtual bool playing();

  virtual void update();

  virtual void set_looping(bool looping);
  virtual void set_relative(bool relative);
  virtual void set_gain(float gain);
  virtual void set_pitch(float pitch);
  virtual void set_position(const Vector& position);
  virtual void set_velocity(const Vector& position);
  virtual void set_reference_distance(float distance);

protected:
  friend class SoundManager;

  ALuint source;
};

#endif

/* EOF */
