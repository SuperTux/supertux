//  $Id: sound_manager.h 2353 2005-04-06 23:00:16Z matzebraun $
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
#ifndef SUPERTUX_SOUND_MANAGER_H
#define SUPERTUX_SOUND_MANAGER_H

#include <string>
#include <vector>
#include <map>

#include "SDL_mixer.h"
#include "math/vector.h"

using namespace SuperTux;

class MusicRef;
class MovingObject;

/** Sound manager
 * This class handles all sounds that are played
 */
class SoundManager
{
public:
  SoundManager();
  ~SoundManager();

  /// Play sound.
  void play_sound(const std::string& sound);
  /// Play sound relative to two Vectors.
  void play_sound(const std::string& sound, const Vector& pos,
      const Vector& pos2);
  /// Play sound relative to a MovingObject and a Vector.
  void play_sound(const std::string& sound, const MovingObject* object,
      const Vector& pos);
  
  /** Load music.
   * Is used to load the music for a MusicRef.
   */
  MusicRef load_music(const std::string& file);

  /**
   * If the sound isn't loaded yet try to load it.
   * Returns an existing instance of the sound, loads a new one and returns that
   * or returns 0 if loading failed.
   */
  Mix_Chunk* preload_sound(const std::string& name);

  /// Test if a certain music file exists.
  bool exists_music(const std::string& filename);

  /** Play music.
   * @param loops: Defaults to -1, which means endless loops.
   */
  void play_music(const MusicRef& music, int loops = -1);

  /// Halt music.
  void halt_music();

  /// Enable/Disable music.
  void enable_music(bool enable);

  /// Is music enabled?
  bool music_enabled()
  {
    return m_music_enabled;
  }

  /// Enable/Disable sound.
  void enable_sound(bool enable);

  /// Is sound enabled?
  bool sound_enabled()
  {
    return m_sound_enabled;
  }

  /// Is audio available?
  bool audio_device_available()
  {
    return audio_device;
  }

  void set_audio_device_available(bool available)
  {
    audio_device = available;
  }

private:
  friend class MusicRef;
  friend class Setup;
  
  /// Resource for music.
  /** Contains the raw music data and
      information for music reference
      counting. */
  class MusicResource
    {
    public:
      ~MusicResource();

      SoundManager* manager;
      Mix_Music* music;
      int refcount;
    };

  void free_music(MusicResource* music);

  typedef std::map<std::string, Mix_Chunk*> Sounds;
  Sounds sounds;

  typedef std::map<std::string, MusicResource> Musics;
  Musics musics;

  MusicResource* current_music;
  bool m_music_enabled;
  bool m_sound_enabled;
  bool audio_device;        /* true: available and initialized */
};

#endif /*SUPERTUX_SOUND_MANAGER_H*/

