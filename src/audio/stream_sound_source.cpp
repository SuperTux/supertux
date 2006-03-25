#include <config.h>
#include <assert.h>

#include <SDL.h>

#include "stream_sound_source.hpp"
#include "sound_manager.hpp"
#include "sound_file.hpp"
#include "msg.hpp"

StreamSoundSource::StreamSoundSource()
  : file(0), fade_state(NoFading), looping(false)
{
  alGenBuffers(STREAMFRAGMENTS, buffers);
  SoundManager::check_al_error("Couldn't allocate audio buffers: ");
}

StreamSoundSource::~StreamSoundSource()
{
  delete file;
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
    SoundManager::check_al_error("Couldn't unqueu audio buffer: ");

    if(fillBufferAndQueue(buffer) == false)
      break;
  }

  if(!playing()) {
    if(processed == 0 || !looping)
      return;
    
    // we might have to restart the source if we had a buffer underrun  
    msg_info("Restarting audio source because of buffer underrun");
    play();
  }

  if(fade_state == FadingOn) {
    Uint32 ticks = SDL_GetTicks();
    float time = (ticks - fade_start_ticks) / 1000.0;
    if(time >= fade_time) {
      set_gain(1.0);
      fade_state = NoFading;
    } else {
      set_gain(time / fade_time);
    }
  } else if(fade_state == FadingOff) {
    Uint32 ticks = SDL_GetTicks();
    float time = (ticks - fade_start_ticks) / 1000.0;
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
  this->fade_start_ticks = SDL_GetTicks();
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
    delete[] bufferdata;
    SoundManager::check_al_error("Couldn't refill audio buffer: ");

    alSourceQueueBuffers(source, 1, &buffer);
    SoundManager::check_al_error("Couldn't queue audio buffer: ");
  }

  // return false if there aren't more buffers to fill
  return bytesread >= STREAMFRAGMENTSIZE;
}

