//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "object/display_effect.hpp"

#include "scripting/squirrel_util.hpp"
#include "supertux/globals.hpp"
#include "video/drawing_context.hpp"

static const float BORDER_SIZE = 75;

DisplayEffect::DisplayEffect(const std::string& name_) :
  ExposedObject<DisplayEffect, scripting::DisplayEffect>(this),
  screen_fade(NO_FADE),
  screen_fadetime(0),
  screen_fading(0),
  border_fade(NO_FADE),
  border_fadetime(0),
  border_fading(),
  border_size(0),
  black(false),
  borders(false)
{
  this->name = name_;
}

DisplayEffect::~DisplayEffect()
{
}

void
DisplayEffect::update(float elapsed_time)
{
  switch(screen_fade) {
    case NO_FADE:
      break;
    case FADE_IN:
      screen_fading -= elapsed_time;
      if(screen_fading < 0) {
        screen_fade = NO_FADE;
      }
      break;
    case FADE_OUT:
      screen_fading -= elapsed_time;
      if(screen_fading < 0) {
        screen_fade = NO_FADE;
        black = true;
      }
      break;
    default:
      assert(false);
  }

  switch(border_fade) {
    case NO_FADE:
      break;
    case FADE_IN:
      border_fading -= elapsed_time;
      if(border_fading < 0) {
        border_fade = NO_FADE;
      }
      border_size = (border_fadetime - border_fading)
        / border_fadetime * BORDER_SIZE;
      break;
    case FADE_OUT:
      border_fading -= elapsed_time;
      if(border_fading < 0) {
        borders = false;
        border_fade = NO_FADE;
      }
      border_size = border_fading / border_fadetime * BORDER_SIZE;
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

  if(black || screen_fade != NO_FADE) {
    float alpha;
    if(black) {
      alpha = 1.0f;
    } else {
      switch(screen_fade) {
        case FADE_IN:
          alpha = screen_fading / screen_fadetime;
          break;
        case FADE_OUT:
          alpha = (screen_fadetime - screen_fading) / screen_fadetime;
          break;
        default:
          alpha = 0;
          assert(false);
      }
    }
    context.draw_filled_rect(Vector(0, 0), Vector(SCREEN_WIDTH, SCREEN_HEIGHT),
                             Color(0, 0, 0, alpha), LAYER_GUI-10);
  }

  if (borders) {
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
  screen_fadetime = fadetime;
  screen_fading = fadetime;
  screen_fade = FADE_OUT;
}

void
DisplayEffect::fade_in(float fadetime)
{
  black = false;
  this->screen_fadetime = fadetime;
  screen_fading = fadetime;
  screen_fade = FADE_IN;
}

void
DisplayEffect::set_black(bool enabled)
{
  black = enabled;
}

bool
DisplayEffect::is_black() const
{
  return black;
}

void
DisplayEffect::sixteen_to_nine(float fadetime)
{
  if(fadetime == 0) {
    borders = true;
    border_size = BORDER_SIZE;
  } else {
    borders = true;
    border_size = 0;
    border_fade = FADE_IN;
    border_fadetime = fadetime;
    border_fading = border_fadetime;
  }
}

void
DisplayEffect::four_to_three(float fadetime)
{
  if(fadetime == 0) {
    borders = false;
  } else {
    border_size = BORDER_SIZE;
    border_fade = FADE_OUT;
    border_fadetime = fadetime;
    border_fading = border_fadetime;
  }
}

/* EOF */
