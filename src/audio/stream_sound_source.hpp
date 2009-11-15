//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef __STREAM_SOUND_SOURCE_H__
#define __STREAM_SOUND_SOURCE_H__

#include <stdio.h>
#include <SDL.h>
#include "openal_sound_source.hpp"

class SoundFile;

class StreamSoundSource : public OpenALSoundSource
{
public:
  StreamSoundSource();
  virtual ~StreamSoundSource();

  void set_sound_file(SoundFile* file);

  enum FadeState { NoFading, FadingOn, FadingOff };

  void set_fading(FadeState state, float fadetime);
  FadeState get_fade_state() const
  {
    return fade_state;
  }
  void update();

  void set_looping(bool looping)
  {
    this->looping = looping;
  }
  bool get_looping() const
  {
    return looping;
  }

private:
  static const size_t STREAMBUFFERSIZE = 1024 * 500;
  static const size_t STREAMFRAGMENTS = 5;
  static const size_t STREAMFRAGMENTSIZE
    = STREAMBUFFERSIZE / STREAMFRAGMENTS;

  bool fillBufferAndQueue(ALuint buffer);
  SoundFile* file;
  ALuint buffers[STREAMFRAGMENTS];

  FadeState fade_state;
  float fade_start_time;
  float fade_time;
  bool looping;

private:
  StreamSoundSource(const StreamSoundSource&);
  StreamSoundSource& operator=(const StreamSoundSource&);
};

#endif
