//  $Id$
//
//  SuperTux -  A Jump'n Run
//  Copyright (C) 2000 Bill Kendrick <bill@newbreedsoftware.com>
//  Copyright (C) 2004 Duong-Khang NGUYEN <neoneurone@users.sf.net>
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
#ifndef HEADER_MUSIC_MANAGER_H
#define HEADER_MUSIC_MANAGER_H
#ifdef GP2X
#include "mikmod.h"
#endif

#include <SDL_mixer.h>
#include <string>
#include <map>

class MusicRef;

/** This class manages a list of music resources and is responsible for playing
 * the music.
 */
class MusicManager
{
public:
  MusicManager();
  ~MusicManager();
    
  MusicRef load_music(const std::string& file);
  bool exists_music(const std::string& filename);
  
  void play_music(const MusicRef& music, int loops = -1);
  void halt_music();

  void enable_music(bool enable);

private:
  friend class MusicRef;
  class MusicResource
  {
  public:
    ~MusicResource();

    MusicManager* manager;
#ifndef GP2X
    Mix_Music* music;
#else
    MODULE *music;
#endif
    
    int refcount;
  };

  void free_music(MusicResource* music);

  std::map<std::string, MusicResource> musics;
  MusicResource* current_music;
  bool music_enabled;
};

#endif

