//  $Id$
//
//  SuperTux -  A Jump'n Run
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

#ifndef SUPERTUX_MUSICREF_H
#define SUPERTUX_MUSICREF_H

#include "audio/sound_manager.h"

/** This class holds a reference to a music file and maintains a correct
 * refcount for that file.
 */
class MusicRef
{
public:
  MusicRef();
  MusicRef(const MusicRef& other);
  ~MusicRef();

  MusicRef& operator= (const MusicRef& other);

private:
  friend class SoundManager;
  MusicRef(SoundManager::MusicResource* music);
  
  SoundManager::MusicResource* music;
};

#endif /*SUPERTUX_MUSICREF_H*/

