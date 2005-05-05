#include <config.h>

#include <string>
#include <stdio.h>
#include "sound.h"
#include "resources.h"
#include "audio/sound_manager.h"

#define NOIMPL      printf("%s not implemented.\n", __PRETTY_FUNCTION__);

namespace Scripting
{

  Sound::Sound()
  {}

  Sound::~Sound() 
  {}

  void
  Sound::play_music(const std::string& )
  {
    NOIMPL;
  }

  void
  Sound::play_sound(const std::string& name)
  {
    sound_manager->play_sound(name);
  }
}
