//  SuperTux
//  Copyright (C) 2021 Ingo Ruhnke <grumbel@gmail.com>
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

#include "audio/sound_source.hpp"

#include <wstsound/sound_source.hpp>

#include "math/vector.hpp"

SoundSource::SoundSource(wstsound::SoundSourcePtr source) :
  m_source(std::move(source))
{
}

void
SoundSource::play()
{
  m_source->play();
}

void
SoundSource::stop()
{
  m_source->stop();
}

bool
SoundSource::playing() const
{
  return m_source->is_playing();
 }

void
SoundSource::set_pitch(float pitch)
{
  m_source->set_pitch(pitch);
}

void
SoundSource::set_position(const Vector& pos)
{
  m_source->set_position(pos.x, pos.y, 0.0f);
}

void
SoundSource::set_looping(bool looping)
{
  m_source->set_looping(looping);
}

void
SoundSource::set_gain(float gain)
{
  m_source->set_gain(gain);
}

void
SoundSource::set_reference_distance(float distance)
{
  m_source->set_reference_distance(distance);
}

/* EOF */
