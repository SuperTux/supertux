//  $Id$
//
//  SuperTux -  A Jump'n Run
//  Copyright (C) 2000 Bill Kendrick <bill@newbreedsoftware.com>
//  Copyright (C) 2004 Matthias Braun <matze@braunis.de>
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
#include "musicref.h"

MusicRef::MusicRef()
  : music(0)
{
}

MusicRef::MusicRef(MusicManager::MusicResource* newmusic)
  : music(newmusic)
{
  if(music)
    music->refcount++;
}

MusicRef::~MusicRef()
{
  if(music) {
    music->refcount--;
    if(music->refcount == 0)
      music->manager->free_music(music);
  }
}

MusicRef::MusicRef(const MusicRef& other)
  : music(other.music)
{
  if(music)
    music->refcount++;
}

MusicRef&
MusicRef::operator =(const MusicRef& other)
{
  MusicManager::MusicResource* oldres = music;
  music = other.music;
  if(music)
    music->refcount++;
  if(oldres) {
    oldres->refcount--;
    if(oldres->refcount == 0)
      music->manager->free_music(music);
  }

  return *this;
}

