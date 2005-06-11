#ifndef __SOUND_H__
#define __SOUND_H__

namespace Scripting
{

/**
 * This class allows manipulating the sound output of the game
 */
class Sound
{
public:
  void play_music(const std::string& musicfile);
  /**
   * Play a sound effect. The name should be without path or .wav extension
   */
  void play_sound(const std::string& soundfile);

#ifndef SCRIPTING_API
  Sound();
  ~Sound();
#endif
};

}

#endif

