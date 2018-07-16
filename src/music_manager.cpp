//  $Id$
//
//  SuperTux
//  Copyright (C) 2004 Ingo Ruhnke <grumbel@gmx.de>
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

#include <assert.h>
#include "music_manager.h"
#include "musicref.h"
#include "sound.h"
#include "setup.h"

MusicManager::MusicManager()
  : current_music(0), music_enabled(true)
{ }

MusicManager::~MusicManager()
{
  if(audio_device)
    Mix_HaltMusic();
}

MusicRef
MusicManager::load_music(const std::string& file)
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
MusicManager::exists_music(const std::string& file)
{
  if(!audio_device)
    return true;
  
  // song already loaded?
  std::map<std::string, MusicResource>::iterator i = musics.find(file);
  if(i != musics.end()) {
    return true;                                      
  }
  
#ifndef GP2X
  Mix_Music* song = Mix_LoadMUS(file.c_str());
#else
  char mfile[100];
  snprintf(mfile,sizeof(mfile),"%s",file.c_str());
  MODULE *song=Player_Load(mfile, 64, 0);
#endif
        
  if(song == 0)
    return false;

  // insert into music list
  std::pair<std::map<std::string, MusicResource>::iterator, bool> result = 
    musics.insert(
      std::pair<std::string, MusicResource>(file, MusicResource()));
  MusicResource& resource = result.first->second;
  resource.manager = this;
  resource.music = song;

  return true;
}

void
MusicManager::free_music(MusicResource* )
{
  // TODO free music, currently we can't do this since SDL_mixer seems to have
  // some bugs if you load/free alot of mod files.  
}

void
MusicManager::play_music(const MusicRef& musicref, int loops)
{
// printf("loop: %d, musicref: %d\n",loops,musicref.music);
  
  if(!audio_device)
    return;

  if(musicref.music == 0 || current_music == musicref.music)
    return;

  if(current_music)
    current_music->refcount--;
  
  current_music = musicref.music;
  current_music->refcount++;
  
  if(music_enabled)
#ifndef GP2X
    Mix_PlayMusic(current_music->music, loops);
#else
  {
    if ( loops == -1 ) current_music->music->wrap=1;
    Player_Stop();
    Player_Start(current_music->music);
    Player_SetPosition(0);
  }
#endif
}

void
MusicManager::halt_music()
{
  if(!audio_device)
    return;
  
#ifndef GP2X
  Mix_HaltMusic();
#else
  Player_Stop();
#endif
  
  if(current_music) {
    current_music->refcount--;
    if(current_music->refcount == 0)
      free_music(current_music);
      current_music = 0;
  }
}

void
MusicManager::enable_music(bool enable)
{
  if(!audio_device)
    return;

  if(enable == music_enabled)
    return;
  
  music_enabled = enable;
  if(music_enabled == false) {
#ifndef GP2X
    Mix_HaltMusic();
#else
    Player_Stop();
#endif
  } else {
#ifndef GP2X
    Mix_PlayMusic(current_music->music, -1);
#else
    Player_Start(current_music->music);
#endif
  }
}

MusicManager::MusicResource::~MusicResource()
{
  // buggy SDL_mixer :-/
  // Mix_FreeMusic(music);
}

