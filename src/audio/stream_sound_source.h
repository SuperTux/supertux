#ifndef __STREAM_SOUND_SOURCE_H__
#define __STREAM_SOUND_SOURCE_H__

#include <stdio.h>
#include "sound_source.h"

class SoundFile;

class StreamSoundSource : public SoundSource
{
public:
  StreamSoundSource(SoundFile* file);
  virtual ~StreamSoundSource();

  void update();
  
private:
  static const size_t STREAMBUFFERSIZE = 1024 * 500;
  static const size_t STREAMFRAGMENTS = 5;
  static const size_t STREAMFRAGMENTSIZE 
    = STREAMBUFFERSIZE / STREAMFRAGMENTS;

  void fillBufferAndQueue(ALuint buffer);
  SoundFile* file;
  ALuint buffers[STREAMFRAGMENTS];
  ALenum format;

  enum FadeState { NoFading, FadingOn, FadingOff };
  FadeState fade_state;
  // TODO
};

#endif

