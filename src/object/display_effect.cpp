#include <config.h>
#include "display_effect.h"

#include <assert.h>
#include "video/drawing_context.h"
#include "main.h"

DisplayEffect::DisplayEffect()
    : type(NO_FADE), fadetime(0), fading(0), black(false)
{
}

DisplayEffect::~DisplayEffect()
{
}

void
DisplayEffect::update(float elapsed_time)
{
    switch(type) {
        case NO_FADE:
            return;
        case FADE_IN:
            fading -= elapsed_time;
            if(fading < 0) {
                type = NO_FADE;
            }
            break;
        case FADE_OUT:
            fading -= elapsed_time;
            if(fading < 0) {
                type = NO_FADE;
                black = true;
            }
            break;
        default:
            assert(false);
    }
}

void
DisplayEffect::draw(DrawingContext& context)
{
    if(!black && type == NO_FADE)
        return;
    
    context.push_transform();
    context.set_translation(Vector(0, 0));

    uint8_t alpha;
    if(black) {
        alpha = 255;
    } else {
        switch(type) {
            case FADE_IN:
                alpha = static_cast<uint8_t>
                    (fading * 255.0 / fadetime);
                break;
            case FADE_OUT:
                alpha = static_cast<uint8_t>
                    ((fadetime-fading) * 255.0 / fadetime);
                break;
            default:
                alpha = 0;
                assert(false);
        }
    }
    context.draw_filled_rect(Vector(0, 0), Vector(SCREEN_WIDTH, SCREEN_HEIGHT),
            Color(0, 0, 0, alpha), LAYER_GUI-10);
    context.pop_transform();
}

void
DisplayEffect::fade_out(float fadetime)
{
    black = false;
    this->fadetime = fadetime;
    fading = fadetime;
    type = FADE_OUT;
}

void
DisplayEffect::fade_in(float fadetime)
{
    black = false;
    this->fadetime = fadetime;
    fading = fadetime;
    type = FADE_IN;
}

void
DisplayEffect::set_black(bool enabled)
{
    black = enabled;
}

bool
DisplayEffect::is_black()
{
    return black;
}

