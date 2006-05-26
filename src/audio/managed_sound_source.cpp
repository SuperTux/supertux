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
  : name(name), soundManager(sound_manager)
{
  soundSource = soundManager->create_sound_source(name);
  if(!soundSource) {
    log_warning << "Couldn't create managed sound source for \"" << name << "\"" << std::endl;
    return;
  }
}

ManagedSoundSource::ManagedSoundSource(const ManagedSoundSource& other) 
	: name(other.name), soundManager(other.soundManager)
{
  soundSource = soundManager->create_sound_source(name);
}

ManagedSoundSource::~ManagedSoundSource()
{
  delete soundSource;
}

