#include <config.h>
#include "display_effect.hpp"

#include <assert.h>
#include "video/drawing_context.hpp"
#include "main.hpp"

DisplayEffect::DisplayEffect()
    : type(NO_FADE), fadetime(0), fading(0), black(false)
{
  cutscene_borders = false;
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
    context.push_transform();
    context.set_translation(Vector(0, 0));

    if(black || type != NO_FADE) {    
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
    }

    if (cutscene_borders) {
      context.draw_filled_rect(Vector(0, 0), Vector(SCREEN_WIDTH, 75),
              Color(0, 0, 0, 255), LAYER_GUI-10);
      context.draw_filled_rect(Vector(0, SCREEN_HEIGHT - 75), Vector(SCREEN_WIDTH, 75),
              Color(0, 0, 0, 255), LAYER_GUI-10);
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
  cutscene_borders = true;
}

void
DisplayEffect::four_to_three()
{
  cutscene_borders = false;
}
