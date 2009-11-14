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

#include <config.h>
#include <assert.h>

#include <SDL.h>

#include "stream_sound_source.hpp"
#include "sound_manager.hpp"
#include "sound_file.hpp"
#include "timer.hpp"
#include "log.hpp"

StreamSoundSource::StreamSoundSource()
  : file(0), fade_state(NoFading), looping(false)
{
  alGenBuffers(STREAMFRAGMENTS, buffers);
  SoundManager::check_al_error("Couldn't allocate audio buffers: ");
  //add me to update list
  sound_manager->register_for_update( this );
}

StreamSoundSource::~StreamSoundSource()
{
  //don't update me any longer
  sound_manager->remove_from_update( this );
  delete file;
  stop();
  alDeleteBuffers(STREAMFRAGMENTS, buffers);
  SoundManager::check_al_error("Couldn't delete audio buffers: ");
}

void
StreamSoundSource::set_sound_file(SoundFile* newfile)
{
  delete file;
  file = newfile;

  ALint queued;
  alGetSourcei(source, AL_BUFFERS_QUEUED, &queued);
  for(size_t i = 0; i < STREAMFRAGMENTS - queued; ++i) {
    if(fillBufferAndQueue(buffers[i]) == false)
      break;
  }
}

void
StreamSoundSource::update()
{
  ALint processed = 0;
  alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
  for(ALint i = 0; i < processed; ++i) {
    ALuint buffer;
    alSourceUnqueueBuffers(source, 1, &buffer);
    SoundManager::check_al_error("Couldn't unqueue audio buffer: ");

    if(fillBufferAndQueue(buffer) == false)
      break;
  }

  if(!playing()) {
    if(processed == 0 || !looping)
      return;

    // we might have to restart the source if we had a buffer underrun
    log_info << "Restarting audio source because of buffer underrun" << std::endl;
    play();
  }

  if(fade_state == FadingOn) {
    float time = real_time - fade_start_time;
    if(time >= fade_time) {
      set_gain(1.0);
      fade_state = NoFading;
    } else {
      set_gain(time / fade_time);
    }
  } else if(fade_state == FadingOff) {
    float time = real_time - fade_start_time;
    if(time >= fade_time) {
      stop();
      fade_state = NoFading;
    } else {
      set_gain( (fade_time-time) / fade_time);
    }
  }
}

void
StreamSoundSource::set_fading(FadeState state, float fade_time)
{
  this->fade_state = state;
  this->fade_time = fade_time;
  this->fade_start_time = real_time;
}

bool
StreamSoundSource::fillBufferAndQueue(ALuint buffer)
{
  // fill buffer
  char* bufferdata = new char[STREAMFRAGMENTSIZE];
  size_t bytesread = 0;
  do {
    bytesread += file->read(bufferdata + bytesread,
        STREAMFRAGMENTSIZE - bytesread);
    // end of sound file
    if(bytesread < STREAMFRAGMENTSIZE) {
      if(looping)
        file->reset();
      else
        break;
    }
  } while(bytesread < STREAMFRAGMENTSIZE);

  if(bytesread > 0) {
    ALenum format = SoundManager::get_sample_format(file);
    alBufferData(buffer, format, bufferdata, bytesread, file->rate);
    SoundManager::check_al_error("Couldn't refill audio buffer: ");

    alSourceQueueBuffers(source, 1, &buffer);
    SoundManager::check_al_error("Couldn't queue audio buffer: ");
  }
  delete[] bufferdata;

  // return false if there aren't more buffers to fill
  return bytesread >= STREAMFRAGMENTSIZE;
}
