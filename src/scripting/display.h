#ifndef __DISPLAY_H__
#define __DISPLAY_H__

namespace Scripting
{

class Display
{
public:
#ifndef SCRIPTING_API
    Display();
    ~Display();
#endif

    void set_effect(const std::string& effect);
};

}

#endif

