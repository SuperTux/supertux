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

#include "supertux/info_box.hpp"

#include "supertux/main.hpp"
#include "supertux/info_box_line.hpp"
#include "util/log.hpp"
#include "video/drawing_context.hpp"
#include "video/surface.hpp"

InfoBox::InfoBox(const std::string& text) :
  firstline(0),
  lines(),
  images(),
  arrow_scrollup(),
  arrow_scrolldown()
{
  // Split text string lines into a vector
  lines = InfoBoxLine::split(text, 400);

  try
  {
    // get the arrow sprites
    arrow_scrollup   = new Surface("images/engine/menu/scroll-up.png");
    arrow_scrolldown = new Surface("images/engine/menu/scroll-down.png");
  }
  catch (std::exception& e)
  {
    log_warning << "Could not load scrolling images: " << e.what() << std::endl;
    arrow_scrollup = 0;
    arrow_scrolldown = 0;
  }
}

InfoBox::~InfoBox()
{
  for(std::vector<InfoBoxLine*>::iterator i = lines.begin();
      i != lines.end(); i++)
    delete *i;

  delete arrow_scrollup;
  delete arrow_scrolldown;
}

void
InfoBox::draw(DrawingContext& context)
{
  float x1 = SCREEN_WIDTH/2-200;
  float y1 = SCREEN_HEIGHT/2-200;
  float width = 400;
  float height = 200;

  context.draw_filled_rect(Vector(x1, y1), Vector(width, height),
                           Color(0.6f, 0.7f, 0.8f, 0.5f), LAYER_GUI-1);

  float y = y1;
  bool linesLeft = false;
  for(size_t i = firstline; i < lines.size(); ++i) {
    if(y >= y1 + height) {
      linesLeft = true;
      break;
    }

    lines[i]->draw(context, Rect(x1, y, x1+width, y), LAYER_GUI);
    y += lines[i]->get_height();
  }

  {
    // draw the scrolling arrows
    if (arrow_scrollup && firstline > 0)
      context.draw_surface(arrow_scrollup,
                           Vector( x1 + width  - arrow_scrollup->get_width(),  // top-right corner of box
                                   y1), LAYER_GUI);

    if (arrow_scrolldown && linesLeft && firstline < lines.size()-1)
      context.draw_surface(arrow_scrolldown,
                           Vector( x1 + width  - arrow_scrolldown->get_width(),  // bottom-light corner of box
                                   y1 + height - arrow_scrolldown->get_height()),
                           LAYER_GUI);
  }
}

void
InfoBox::scrollup()
{
  if(firstline > 0)
    firstline--;
}

void
InfoBox::scrolldown()
{
  if(firstline < lines.size()-1)
    firstline++;
}

void
InfoBox::pageup()
{
}

void
InfoBox::pagedown()
{
}

/* EOF */
