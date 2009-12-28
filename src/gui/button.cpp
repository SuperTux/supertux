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

#include "gui/button.hpp"

#include "supertux/globals.hpp"
#include "video/drawing_context.hpp"

FontPtr Button::info_font;

Button::Button(SurfacePtr image_, std::string info_, SDLKey binding_) :
  pos(),
  size(),
  image(),
  binding(binding_),
  id(),
  state(),
  info()
{
  image = image_;
  size = Vector(image->get_width(), image->get_height());
  id = 0;
  info = info_;
}

Button::Button(const Button& rhs) :
  pos(rhs.pos),
  size(rhs.size),
  image(rhs.image),
  binding(rhs.binding),
  id(rhs.id),
  state(rhs.state),
  info(rhs.info)
{
}

Button::~Button()
{
}

Button&
Button::operator=(const Button& rhs)
{
  if (this != &rhs)
  {
    pos = rhs.pos;
    size = rhs.size;
    image = rhs.image;
    binding = rhs.binding;
    id = rhs.id;
    state = rhs.state;
    info = rhs.info;
  }
  return *this;
}

void Button::draw(DrawingContext &context, bool selected)
{
  if(selected)
    context.draw_filled_rect(pos, size, Color (200,240,220), LAYER_GUI);
  else
    context.draw_filled_rect(pos, size, Color (200,200,220), LAYER_GUI);

  Vector tanslation = -context.get_translation();
  if(state == BT_SHOW_INFO)
  {
    Vector offset;
    if(pos.x + tanslation.x < 100 && pos.y + tanslation.y > SCREEN_SIZE.height - 20)
      offset = Vector(size.x, - 10);
    else if(pos.x + tanslation.x < 100)
      offset = Vector(size.x, 0);
    else
      offset = Vector(-30, -size.y/2);
    context.draw_text(info_font, info, pos + offset, ALIGN_LEFT, LAYER_GUI+2);
    if(binding != 0)
      context.draw_text(info_font, "(" + std::string(SDL_GetKeyName(binding)) +
                        ")", pos + offset + Vector(0,12),
                        ALIGN_LEFT,  LAYER_GUI+2);
  }

  context.draw_surface_part(image, Vector(0,0), size, pos, LAYER_GUI+1);
}

int Button::event(SDL_Event &event, int x_offset, int y_offset)
{
  state = BT_NONE;
  switch(event.type)
  {
    case SDL_MOUSEBUTTONDOWN:
      if(event.button.x > pos.x + x_offset && event.button.x < pos.x + x_offset + size.x &&
         event.button.y > pos.y + y_offset && event.button.y < pos.y + y_offset + size.y)
      {
        if(event.button.button == SDL_BUTTON_RIGHT)
          state = BT_SHOW_INFO;
      }
      break;
    case SDL_MOUSEBUTTONUP:
      if(event.button.x > pos.x + x_offset && event.button.x < pos.x + x_offset + size.x &&
         event.button.y > pos.y + y_offset && event.button.y < pos.y + y_offset + size.y)
      {
        if(event.button.button == SDL_BUTTON_LEFT)
          state = BT_SELECTED;
      }
      break;
    case SDL_KEYDOWN:        // key pressed
      if(event.key.keysym.sym == binding)
        state = BT_SELECTED;
      break;
    default:
      break;
  }
  return state;
}

/* EOF */
