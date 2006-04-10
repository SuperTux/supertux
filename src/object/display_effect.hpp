#ifndef __OBJECT_DISPLAY_EFFECT_H__
#define __OBJECT_DISPLAY_EFFECT_H__

#include "scripting/display_effect.hpp"
#include "game_object.hpp"
#include "script_interface.hpp"

class DisplayEffect : public GameObject, public Scripting::DisplayEffect,
                      public ScriptInterface
{
public:
    DisplayEffect();
    virtual ~DisplayEffect();

    void expose(HSQUIRRELVM vm, int table_idx);
    void unexpose(HSQUIRRELVM vm, int table_idx);

    void update(float elapsed_time);
    void draw(DrawingContext& context);

    void fade_out(float fadetime);
    void fade_in(float fadetime);
    void set_black(bool enabled);
    bool is_black();
    void sixteen_to_nine(float fadetime);
    void four_to_three(float fadetime);

private:
    enum FadeType {
        NO_FADE, FADE_IN, FADE_OUT
    };
    FadeType screen_fade;
    float screen_fadetime;
    float screen_fading;
    FadeType border_fade;
    float border_fadetime;
    float border_fading;
    float border_size;
    
    bool black;
    bool borders;
};

#endif

