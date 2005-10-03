#include <config.h>
#include "display_effect.hpp"

#include <assert.h>
#include "video/drawing_context.hpp"
#include "main.hpp"

DisplayEffect::DisplayEffect()
    : type(NO_FADE), fadetime(0), fading(0), black(false)
{
 border_size = 0; 
 borders_fading = false;
 borders_active = false;
}

DisplayEffect::~DisplayEffect()
{
}

void
DisplayEffect::update(float elapsed_time)
{
    if (borders_fading) {
      if (border_size < 75) border_size += 1.5;
      else borders_active = true;
    }
    else if (borders_active) {
      if (border_size > 0) border_size -= 1.5;
      else borders_active = false;
    }
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
    context.push_transform();
    context.set_translation(Vector(0, 0));

    if(black || type != NO_FADE) {    
      float alpha;
      if(black) {
          alpha = 1.0f;
      } else {
          switch(type) {
              case FADE_IN:
                  alpha = fading / fadetime;
                  break;
              case FADE_OUT:
                  alpha = (fadetime-fading) / fadetime;
                  break;
              default:
                  alpha = 0;
                  assert(false);
          }
      }
      context.draw_filled_rect(Vector(0, 0), Vector(SCREEN_WIDTH, SCREEN_HEIGHT),
              Color(0, 0, 0, alpha), LAYER_GUI-10);
    }

    if (borders_fading || borders_active) {
      context.draw_filled_rect(Vector(0, 0), Vector(SCREEN_WIDTH, border_size),
              Color(0, 0, 0, 1.0f), LAYER_GUI-10);
      context.draw_filled_rect(Vector(0, SCREEN_HEIGHT - border_size), Vector(SCREEN_WIDTH, border_size),
              Color(0, 0, 0, 1.0f), LAYER_GUI-10);
    }

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

void
DisplayEffect::sixteen_to_nine()
{
  borders_fading = true;
}

void
DisplayEffect::four_to_three()
{
  borders_fading = false;
}

