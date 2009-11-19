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

#include "object/floating_text.hpp"

#include <stdio.h>

#include "supertux/resources.hpp"
#include "video/drawing_context.hpp"

FloatingText::FloatingText(const Vector& pos, const std::string& text_) :
  position(pos), 
  text(text_),
  timer()
{
  timer.start(.1f);
  position.x -= text.size() * 8;
}

FloatingText::FloatingText(const Vector& pos, int score) :
  position(pos),
  text(),
  timer()
{
  timer.start(.1f);

  // turn int into a string
  char str[10];
  snprintf(str, 10, "%d", score);
  text = str;

  position.x -= text.size() * 8;
}

void
FloatingText::update(float elapsed_time)
{
  position.y -= 1.4 * elapsed_time;

  if(timer.check())
    remove_me();
}

#define FADING_TIME .350

void
FloatingText::draw(DrawingContext& context)
{
  // make an alpha animation when disappearing
  int alpha;
  if(timer.get_timeleft() < FADING_TIME)
    alpha = int(timer.get_timeleft() * 255 / FADING_TIME);
  else
    alpha = 255;

  context.push_transform();
  context.set_alpha(alpha);

  context.draw_text(Resources::normal_font, text, position, ALIGN_LEFT, LAYER_OBJECTS+1, FloatingText::text_color);

  context.pop_transform();
}

/* EOF */
