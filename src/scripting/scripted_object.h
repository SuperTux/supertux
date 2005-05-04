#ifndef __SCRIPTED_OBJECT_H__
#define __SCRIPTED_OBJECT_H__

namespace Scripting
{

class ScriptedObject
{
public:
#ifndef SCRIPTING_API
    ScriptedObject();
    virtual ~ScriptedObject();
#endif

    void set_animation(const std::string& animation);
    std::string get_animation();

    void move(float x, float y);
    void set_pos(float x, float y);
    float get_pos_x();
    float get_pos_y();
    void set_velocity(float x, float y);
    float get_velocity_x();
    float get_velocity_y();
};

}

#endif

