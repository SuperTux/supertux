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

#ifndef SUPERTUX_SOUND_MANAGER_H
#define SUPERTUX_SOUND_MANAGER_H

#include <string>
#include <vector>
#include <map>

#include "SDL_mixer.h"
#include "math/vector.h"

namespace SuperTux
  {

  class MusicRef;
  class MovingObject;

  /// enum of different internal music types
  enum Music_Type {
    NO_MUSIC,
    LEVEL_MUSIC,
    HURRYUP_MUSIC,
    HERRING_MUSIC,
    CREDITS_MUSIC
  };

  /// Sound manager
  /** This class handles all sounds that are played
   */
  class SoundManager
    {
    public:
      /// Play sound.
      void play_sound(Mix_Chunk* sound);
      /// Play sound relative to two Vectors.
      void play_sound(Mix_Chunk* sound, const Vector& pos, const Vector& pos2);
      /// Play sound relative to a MovingObject and a Vector.
      void play_sound(Mix_Chunk* sound, const MovingObject* object, const Vector& pos);
      
      /// Load music.
      /** Is used to load the music for a MusicRef. */
      MusicRef load_music(const std::string& file);

      /// Load sound.
      Mix_Chunk * load_sound(const std::string& file);

      /// Test if a certain music file exists.
      bool exists_music(const std::string& filename);

      /// Play music.
      /** @param loops: Defaults to -1, which means endless loops. */
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

      /* functions handling the sound and music */
      int open_audio(int frequency, Uint16 format, int channels, int chunksize);
      void close_audio( void );

      /// Is audio available?
      bool audio_device_available()
      {
        return audio_device;
      }

      void set_audio_device_available(bool available)
      {
        audio_device = available;
      }

      static SoundManager* get()
        {
          return instance_ ? instance_ : instance_ = new SoundManager();
        }
      static void destroy_instance()
      {
        delete instance_;
        instance_ = 0;
      }
      
      void add_sound(Mix_Chunk* sound, int id)
      {
        sounds[id] = sound;
      }
      
      Mix_Chunk* get_sound(int id)
      {
        return sounds[id];
      }

    private:
      SoundManager();
      ~SoundManager();

      // music part
      friend class MusicRef;
      friend class Setup;
      friend Mix_Chunk* IDToSound(int id);
      
      static SoundManager* instance_ ;

      void free_chunk(Mix_Chunk* chunk);

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

      /*variables for stocking the sound and music*/
      std::map<int, Mix_Chunk*> sounds;
      std::map<std::string, MusicResource> musics;
      MusicResource* current_music;
      bool m_music_enabled;
      bool m_sound_enabled;
      bool audio_device;        /* != 0: available and initialized */
    };

   Mix_Chunk* IDToSound(int id);
    
} // namespace SuperTux
#endif /*SUPERTUX_SOUND_MANAGER_H*/

