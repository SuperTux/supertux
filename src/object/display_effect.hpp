#ifndef __OBJECT_DISPLAY_EFFECT_H__
#define __OBJECT_DISPLAY_EFFECT_H__

#include "scripting/display_effect.hpp"
#include "game_object.hpp"

class DisplayEffect : public GameObject, public Scripting::DisplayEffect
{
public:
    DisplayEffect();
    virtual ~DisplayEffect();

    void update(float elapsed_time);
    void draw(DrawingContext& context);

    void fade_out(float fadetime);
    void fade_in(float fadetime);
    void set_black(bool enabled);
    bool is_black();

private:
    enum FadeType {
        NO_FADE, FADE_IN, FADE_OUT
    };
    FadeType type;
    float fadetime;
    float fading;
    bool black;
};

#endif

