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

#include "audio/dummy_sound_source.hpp"
#include "audio/sound_source.hpp"

class DummySoundSource : public SoundSource
{
public:
  DummySoundSource() :
    is_playing()
  {}
  virtual ~DummySoundSource()
  {}

  virtual void play()
  {
    is_playing = true;
  }

  virtual void stop()
  {
    is_playing = false;
  }

  virtual bool playing()
  {
    return is_playing;
  }

  virtual void set_looping(bool )
  {
  }

  virtual void set_gain(float )
  {
  }

  virtual void set_pitch(float )
  {
  }

  virtual void set_position(const Vector& )
  {
  }

  virtual void set_velocity(const Vector& )
  {
  }

  virtual void set_reference_distance(float )
  {
  }

  virtual void set_rollof_factor(float )
  {
  }

private:
  bool is_playing;
};

SoundSource* create_dummy_sound_source()
{
  return new DummySoundSource();
}
