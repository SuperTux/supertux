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

#ifndef __MANAGED_SOUND_SOURCE_H__
#define __MANAGED_SOUND_SOURCE_H__

#include <string>
#include "audio/sound_manager.hpp"
#include "audio/sound_source.hpp"

/**
 * Encapsulates a SoundSource that is managed by a SoundManager
 */
class ManagedSoundSource
{
public:
  ManagedSoundSource(SoundManager* sound_manager, const std::string& name);
  ManagedSoundSource(const ManagedSoundSource& managed_sound_source);
  virtual ManagedSoundSource* clone() const { return new ManagedSoundSource(*this); }
  virtual ~ManagedSoundSource();

  /**
   * pre-loads SoundSource and indicates success.
   * If pre-loading failed, the SoundSource will be loaded at first use
   */
  bool preload();

  /**
   * stops playing and temporarily releases memory for the SoundSource.
   * Memory will be re-allocated on next use
   */
  void release();

  void play();
  void stop();
  bool playing();
  void set_looping(bool looping) { if (preload()) soundSource->set_looping(looping); }
  void set_gain(float gain) { if (preload()) soundSource->set_gain(gain); }
  void set_pitch(float pitch) { if (preload()) soundSource->set_pitch(pitch); }
  void set_position(Vector position) { if (preload()) soundSource->set_position(position); }
  void set_velocity(Vector position) { if (preload()) soundSource->set_velocity(position); }
  void set_reference_distance(float distance) { if (preload()) soundSource->set_reference_distance(distance); }

protected:
  std::string name;
  SoundSource* soundSource;
  SoundManager* soundManager;

private:
};

#endif

