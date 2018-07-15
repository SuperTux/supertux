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

#include "audio/sound_file.hpp"
#include "audio/sound_manager.hpp"
#include "audio/stream_sound_source.hpp"
#include "supertux/globals.hpp"
#include "util/log.hpp"

StreamSoundSource::StreamSoundSource() :
  file(),
  fade_state(NoFading),
  fade_start_time(),
  fade_time(),
  looping(false)
{
  alGenBuffers(STREAMFRAGMENTS, buffers);
  try
  {
    SoundManager::check_al_error("Couldn't allocate audio buffers: ");
  }
  catch(std::exception& e)
  {
    log_warning << e.what() << std::endl;
  }
  //add me to update list
  SoundManager::current()->register_for_update( this );
}

StreamSoundSource::~StreamSoundSource()
{
  //don't update me any longer
  SoundManager::current()->remove_from_update( this );
  file.reset();
  stop();
  alDeleteBuffers(STREAMFRAGMENTS, buffers);
  try
  {
    SoundManager::check_al_error("Couldn't delete audio buffers: ");
  }
  catch(std::exception& e)
  {
    // Am I bovvered?
    log_warning << e.what() << std::endl;
  }
}

void
StreamSoundSource::set_sound_file(std::unique_ptr<SoundFile> newfile)
{
  file = std::move(newfile);

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
    try
    {
      SoundManager::check_al_error("Couldn't unqueue audio buffer: ");
    }
    catch(std::exception& e)
    {
      log_warning << e.what() << std::endl;
    }

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

  if(fade_state == FadingOn || fade_state == FadingResume) {
    float time = real_time - fade_start_time;
    if(time >= fade_time) {
      set_gain(1.0);
      fade_state = NoFading;
    } else {
      set_gain(time / fade_time);
    }
  } else if(fade_state == FadingOff || fade_state == FadingPause) {
    float time = real_time - fade_start_time;
    if(time >= fade_time) {
      if(fade_state == FadingOff)
        stop();
      else
        pause();
      fade_state = NoFading;
    } else {
      set_gain( (fade_time-time) / fade_time);
    }
  }
}

void
StreamSoundSource::set_fading(FadeState state, float fade_time_)
{
  fade_state = state;
  fade_time = fade_time_;
  fade_start_time = real_time;
}

bool
StreamSoundSource::fillBufferAndQueue(ALuint buffer)
{
  // fill buffer
  std::unique_ptr<char[]> bufferdata(new char[STREAMFRAGMENTSIZE]);
  size_t bytesread = 0;
  do {
    bytesread += file->read(bufferdata.get() + bytesread,
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
    ALenum format = SoundManager::get_sample_format(*file);
    try
    {
      alBufferData(buffer, format, bufferdata.get(), bytesread, file->rate);
      SoundManager::check_al_error("Couldn't refill audio buffer: ");

      alSourceQueueBuffers(source, 1, &buffer);
      SoundManager::check_al_error("Couldn't queue audio buffer: ");
    }
    catch(std::exception& e)
    {
      log_warning << e.what() << std::endl;
    }
  }

  // return false if there aren't more buffers to fill
  return bytesread >= STREAMFRAGMENTSIZE;
}

/* EOF */
