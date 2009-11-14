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

class Vector;

/**
 * A sound source represents the source of audio output. You can place
 * sources at certain points in your world or set their velocity to produce
 * doppler effects
 */
class SoundSource
{
public:
  virtual ~SoundSource()
  { }

  virtual void play() = 0;
  virtual void stop() = 0;
  virtual bool playing() = 0;

  virtual void set_looping(bool looping) = 0;
  /// Set volume (0.0 is silent, 1.0 is normal)
  virtual void set_gain(float gain) = 0;
  virtual void set_pitch(float pitch) = 0;
  virtual void set_position(const Vector& position) = 0;
  virtual void set_velocity(const Vector& position) = 0;
  virtual void set_reference_distance(float distance) = 0;
  virtual void set_rollof_factor(float factor) = 0;
};

#endif
