#include <config.h>

#include <string>
#include <stdio.h>
#include "sound.h"
#include "resources.h"
#include "audio/sound_manager.h"

namespace Scripting
{

  Sound::Sound()
  {}

  Sound::~Sound() 
  {}

  void
  Sound::play_music(const std::string& name)
  {
    std::string filename = "music/";
    filename += name;
    filename += ".ogg";
    sound_manager->play_music(name);
  }

  void
  Sound::play(const std::string& name)
  {
    sound_manager->play(name);
  }
}
