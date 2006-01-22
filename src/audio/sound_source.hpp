#ifndef __SOUND_SOURCE_H__
#define __SOUND_SOURCE_H__

#include <AL/al.h>
#include "math/vector.hpp"

class SoundSource
{
public:
  SoundSource();
  virtual ~SoundSource();

  void play();
  void stop();
  bool playing();

  virtual void update();

  void set_looping(bool looping);
  /// Set volume (0.0 is silent, 1.0 is normal)
  void set_gain(float gain);
  void set_position(Vector position);
  void set_velocity(Vector position);
  void set_reference_distance(float distance);

protected:
  friend class SoundManager;
  
  ALuint source;
};

#endif

