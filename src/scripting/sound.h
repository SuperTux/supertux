#ifndef __SOUND_H__
#define __SOUND_H__

namespace Scripting
{

class Sound
{
public:
    void play_music(const std::string& musicfile);
    void play_sound(const std::string& soundfile);

#ifndef SCRIPTING_API
    Sound();
    ~Sound();
#endif
};

}

#endif

