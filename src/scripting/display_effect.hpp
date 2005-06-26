#ifndef __SCRIPTING_DISPLAY_EFFECT_H__
#define __SCRIPTING_DISPLAY_EFFECT_H__

namespace Scripting
{

class DisplayEffect
{
public:
#ifndef SCRIPTING_API
    virtual ~DisplayEffect()
    {}
#endif

    /// fade display to black
    virtual void fade_out(float fadetime) = 0;
    /// fade display from black to normal
    virtual void fade_in(float fadetime) = 0;
    /// set display black (or back to normal)
    virtual void set_black(bool enabled) = 0;
    /// check if display is set to black
    virtual bool is_black() = 0;

    // fade display until just a small visible circle is left
    // (like what happens in some cartoons at the end)
    // void shrink_fade(Vector goal, float radius, float fadetime);
};

}

#endif

