#ifndef __LEVEL_H__
#define __LEVEL_H__

namespace Scripting
{

class Level
{
public:
#ifndef SCRIPTING_API
    Level();
    ~Level();
#endif

    /** Instantly finish the currently played level */
    void finish();
    /** spawn tux at specified sector and spawnpoint */
    void spawn(const std::string& sector, const std::string& spawnpoint);
    /** Flip level vertically */
    void flip_vertically();
};

}

#endif

