#ifndef __STREAM_SOUND_SOURCE_H__
#define __STREAM_SOUND_SOURCE_H__

#include <stdio.h>
#include <SDL.h>
#include "sound_source.hpp"

class SoundFile;

class StreamSoundSource : public SoundSource
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
  
private:
  //static const size_t STREAMBUFFERSIZE = 1024 * 500;
  static const size_t STREAMBUFFERSIZE = 1024 * 20;
  static const size_t STREAMFRAGMENTS = 5;
  static const size_t STREAMFRAGMENTSIZE 
    = STREAMBUFFERSIZE / STREAMFRAGMENTS;

  void fillBufferAndQueue(ALuint buffer);
  SoundFile* file;
  ALuint buffers[STREAMFRAGMENTS];

  FadeState fade_state;
  Uint32 fade_start_ticks;
  float fade_time;
};

#endif

