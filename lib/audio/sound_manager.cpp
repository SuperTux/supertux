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

#include <cmath>
#include <cassert>

#include "audio/sound_manager.h"
#include "audio/musicref.h"
#include "audio/sound.h"
#include "app/globals.h"
#include "app/setup.h"
#include "special/moving_object.h"

using namespace SuperTux;

SoundManager::SoundManager()
  : current_music(0), music_enabled(true)
{
}

SoundManager::~SoundManager()
{
  if(audio_device)
    Mix_HaltMusic();
}

void
SoundManager::play_sound(Mix_Chunk* sound)
{
  if(!audio_device || !use_sound)
    return;

  Mix_PlayChannel(-1, sound, 0);  
}

void
SoundManager::play_sound(Mix_Chunk* sound, const MovingObject* object, const Vector& pos)
{
  // TODO keep track of the object later and move the sound along with the
  // object.
  play_sound(sound, object->get_pos(), pos);
}

void
SoundManager::play_sound(Mix_Chunk* sound, const Vector& pos, const Vector& pos2)
{
  if(!audio_device || !use_sound)
    return;

  // TODO make sure this formula is good
  float distance 
    = pos2.x- pos.x;
  int loud = int(255.0/float(screen->w*2) * fabsf(distance));
  if(loud > 255)
    return;

  int chan = Mix_PlayChannel(-1, sound, 0);
  if(chan < 0)
    return;                                  
  Mix_SetDistance(chan, loud);

  // very bad way to do this...
  if(distance > 100)
    Mix_SetPanning(chan, 230, 24);
  else if(distance < -100)
    Mix_SetPanning(chan, 24, 230);
}

MusicRef
SoundManager::load_music(const std::string& file)
{
  if(!audio_device)
    return MusicRef(0);

  if(!exists_music(file))
    st_abort("Couldn't load musicfile ", file.c_str());

  std::map<std::string, MusicResource>::iterator i = musics.find(file);
  assert(i != musics.end());
  return MusicRef(& (i->second));
}

bool
SoundManager::exists_music(const std::string& file)
{
  if(!audio_device)
    return true;
  
  // song already loaded?
  std::map<std::string, MusicResource>::iterator i = musics.find(file);
  if(i != musics.end()) {
    return true;                                      
  }
  
  Mix_Music* song = Mix_LoadMUS(file.c_str());
  if(song == 0)
    return false;

  // insert into music list
  std::pair<std::map<std::string, MusicResource>::iterator, bool> result = 
    musics.insert(
        std::make_pair<std::string, MusicResource> (file, MusicResource()));
  MusicResource& resource = result.first->second;
  resource.manager = this;
  resource.music = song;

  return true;
}

void
SoundManager::free_music(MusicResource* )
{
  // TODO free music, currently we can't do this since SDL_mixer seems to have
  // some bugs if you load/free alot of mod files.  
}

void
SoundManager::play_music(const MusicRef& musicref, int loops)
{
  if(!audio_device)
    return;

  if(musicref.music == 0 || current_music == musicref.music)
    return;

  if(current_music)
    current_music->refcount--;
  
  current_music = musicref.music;
  current_music->refcount++;
  
  if(music_enabled)
    Mix_PlayMusic(current_music->music, loops);
}

void
SoundManager::halt_music()
{
  if(!audio_device)
    return;
  
  Mix_HaltMusic();
  
  if(current_music) {
    current_music->refcount--;
    if(current_music->refcount == 0)
      free_music(current_music);
    current_music = 0;
  }
}

void
SoundManager::enable_music(bool enable)
{
  if(!audio_device)
    return;

  if(enable == music_enabled)
    return;
  
  music_enabled = enable;
  if(music_enabled == false) {
    Mix_HaltMusic();
  } else {
    Mix_PlayMusic(current_music->music, -1);
  }
}

SoundManager::MusicResource::~MusicResource()
{
  // don't free music buggy SDL_Mixer crashs for some mod files
  // Mix_FreeMusic(music);
}

