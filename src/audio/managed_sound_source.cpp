//  $Id$
// 
//  SuperTux - Managed Sound Source
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.
#include <config.h>

#include "audio/managed_sound_source.hpp"
#include "log.hpp"

ManagedSoundSource::ManagedSoundSource(SoundManager* sound_manager, const std::string& name)
  : name(name), soundSource(0), soundManager(sound_manager)
{
}

ManagedSoundSource::ManagedSoundSource(const ManagedSoundSource& other) 
	: name(other.name), soundSource(0), soundManager(other.soundManager)
{
}

ManagedSoundSource::~ManagedSoundSource()
{
  delete soundSource;
}

bool
ManagedSoundSource::preload()
{
  if (soundSource) return true;
  soundSource = soundManager->create_sound_source(name);
  return (soundSource != 0);
}

void
ManagedSoundSource::release()
{
  if (!soundSource) return;
  if (playing()) soundSource->stop();
  delete soundSource;
  soundSource = 0;
}

void
ManagedSoundSource::play()
{
  if (!preload()) {
    log_warning << "Couldn't play \"" << name << "\"" << std::endl;
    return;
  }
  soundSource->play();
}

void
ManagedSoundSource::stop()
{
  // FIXME: calling release() instead of stop() seems necessary due to an unconfirmed sound bug
  release();
}

bool
ManagedSoundSource::playing()
{
  if (!soundSource) return false;
  return soundSource->playing();
}

