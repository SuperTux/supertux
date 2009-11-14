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

#ifndef __SOUND_FILE_H__
#define __SOUND_FILE_H__

#include <stdio.h>
#include <iostream>

class SoundFile
{
public:
  virtual ~SoundFile()
  { }

  virtual size_t read(void* buffer, size_t buffer_size) = 0;
  virtual void reset() = 0;

  int channels;
  int rate;
  int bits_per_sample;
  /// size in bytes
  size_t size;
};

SoundFile* load_sound_file(const std::string& filename);

#endif
