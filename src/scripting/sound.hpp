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
  void play(const std::string& soundfile);

  ~Sound();

#ifndef SCRIPTING_API
  Sound();
#endif
};

}

#endif

