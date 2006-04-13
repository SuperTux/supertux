//  $Id$
//
//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include <config.h>

#include "sound_source.hpp"
#include "sound_manager.hpp"

SoundSource::SoundSource()
{
  alGenSources(1, &source);
  SoundManager::check_al_error("Couldn't create audio source: ");
  set_reference_distance(128);
}

SoundSource::~SoundSource()
{
  stop();
  alDeleteSources(1, &source);
}

void
SoundSource::stop()
{
  alSourceStop(source);
  alSourcei(source, AL_BUFFER, AL_NONE);
  SoundManager::check_al_error("Problem stopping audio source: ");
}

void
SoundSource::play()
{
  alSourcePlay(source);
  SoundManager::check_al_error("Couldn't start audio source: ");
}

bool
SoundSource::playing()
{
  ALint state = AL_PLAYING;
  alGetSourcei(source, AL_SOURCE_STATE, &state);
  return state != AL_STOPPED;
}

void
SoundSource::update()
{
}

void
SoundSource::set_looping(bool looping)
{
  alSourcei(source, AL_LOOPING, looping ? AL_TRUE : AL_FALSE);
}

void
SoundSource::set_position(Vector position)
{
  alSource3f(source, AL_POSITION, position.x, position.y, 0);
}

void
SoundSource::set_velocity(Vector velocity)
{
  alSource3f(source, AL_VELOCITY, velocity.x, velocity.y, 0);
}

void
SoundSource::set_gain(float gain)
{
  alSourcef(source, AL_GAIN, gain);
}

void
SoundSource::set_reference_distance(float distance)
{
  alSourcef(source, AL_REFERENCE_DISTANCE, distance);
}
