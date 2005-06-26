#include <config.h>

#include "stream_sound_source.hpp"
#include "sound_manager.hpp"
#include "sound_file.hpp"

StreamSoundSource::StreamSoundSource(SoundFile* file)
{
  this->file = file;
  alGenBuffers(STREAMFRAGMENTS, buffers);
  SoundManager::check_al_error("Couldn't allocate audio buffers: ");
  format = SoundManager::get_sample_format(file);
  try {
    for(size_t i = 0; i < STREAMFRAGMENTS; ++i) {
      fillBufferAndQueue(buffers[i]);
    }
  } catch(...) {
    alDeleteBuffers(STREAMFRAGMENTS, buffers);
  }
}

StreamSoundSource::~StreamSoundSource()
{
  alDeleteBuffers(STREAMFRAGMENTS, buffers);
  SoundManager::check_al_error("Couldn't delete audio buffers: ");
}

void
StreamSoundSource::update()
{
  if(!playing())
    return;

  ALint processed = 0;
  alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
  while(processed > 0) {
    processed--;

    ALuint buffer;
    alSourceUnqueueBuffers(source, 1, &buffer);
    SoundManager::check_al_error("Couldn't unqueu audio buffer: ");

    fillBufferAndQueue(buffer);
  }
  
  // we might have to restart the source if we had a buffer underrun
  if(!playing()) {
    std::cerr << "Restarting audio source because of buffer underrun.\n";
    alSourcePlay(source);
    SoundManager::check_al_error("Couldn't restart audio source: ");
  }

  // TODO handle fading
}

void
StreamSoundSource::fillBufferAndQueue(ALuint buffer)
{
  // fill buffer
  char* bufferdata = new char[STREAMFRAGMENTSIZE];
  size_t bytesread = 0;
  do {
    bytesread += file->read(bufferdata + bytesread,
        STREAMFRAGMENTSIZE - bytesread);
    if(bytesread < STREAMFRAGMENTSIZE) {
      file->reset();
    }
  } while(bytesread < STREAMFRAGMENTSIZE);
  
  alBufferData(buffer, format, bufferdata, STREAMFRAGMENTSIZE, file->rate);
  delete[] bufferdata;
  SoundManager::check_al_error("Couldn't refill audio buffer: ");

  alSourceQueueBuffers(source, 1, &buffer);
  SoundManager::check_al_error("Couldn't queue audio buffer: ");
}
