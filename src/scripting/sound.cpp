#include <config.h>

#include <string>
#include <stdio.h>
#include "sound.hpp"
#include "resources.hpp"
#include "audio/sound_manager.hpp"

namespace Scripting
{

  Sound::Sound()
  {}

  Sound::~Sound() 
  {}

  void
  Sound::play_music(const std::string& filename)
  {
    sound_manager->play_music(filename);
  }

  void
  Sound::play(const std::string& name)
  {
    sound_manager->play(name);
  }
}
