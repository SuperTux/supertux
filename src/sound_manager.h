//  $Id$
//
//  SuperTux -  A Jump'n Run
//  Copyright (C) 2004 Matthias Braun <matze@braunis.de
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
#ifndef __SOUND_MANAGER_H__
#define __SOUND_MANAGER_H__

#include "vector.h"
#include <SDL_mixer.h>
#include <string>
#include <map>

class MusicRef;
class MovingObject;

/** This class handles all sounds that are played
 */
class SoundManager
{
public:
  SoundManager();
  ~SoundManager();

  void play_sound(Mix_Chunk* sound);
  void play_sound(Mix_Chunk* sound, const Vector& pos);
  void play_sound(Mix_Chunk* sound, const MovingObject* object);

  MusicRef load_music(const std::string& file);
  bool exists_music(const std::string& filename);
  
  void play_music(const MusicRef& music, int loops = -1);
  void halt_music();

  void enable_music(bool enable);

private:
  // music part
  friend class MusicRef;
  class MusicResource
  {
  public:
    ~MusicResource();

    SoundManager* manager;
    Mix_Music* music;
    int refcount;
  };

  void free_music(MusicResource* music);

  std::map<std::string, MusicResource> musics;
  MusicResource* current_music;
  bool music_enabled;
};

#endif

