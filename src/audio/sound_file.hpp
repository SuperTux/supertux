#ifndef __SOUND_FILE_H__
#define __SOUND_FILE_H__

#include <stdio.h>
#include <iostream>

class SoundFile
{
public:
  virtual ~SoundFile()
  { }

  virtual size_t read(void* buffer, size_t buffer_size) = 0;
  virtual void reset() = 0;

  int channels;
  int rate;
  int bits_per_sample;
  /// size in bytes
  size_t size;
};

SoundFile* load_sound_file(const std::string& filename);

#endif

