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

#ifndef __SOUND_SOURCE_H__
#define __SOUND_SOURCE_H__

#include <AL/al.h>
#include "math/vector.hpp"

class SoundSource
{
public:
  SoundSource();
  virtual ~SoundSource();

  void play();
  void stop();
  bool playing();

  virtual void update();

  void set_looping(bool looping);
  /// Set volume (0.0 is silent, 1.0 is normal)
  void set_gain(float gain);
  void set_pitch(float pitch);
  void set_position(Vector position);
  void set_velocity(Vector position);
  void set_reference_distance(float distance);

protected:
  friend class SoundManager;
  
  ALuint source;
};

#endif

