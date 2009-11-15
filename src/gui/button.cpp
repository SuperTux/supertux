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

#include "supertux/main.hpp"
#include "video/drawing_context.hpp"

Font* Button::info_font = 0;
extern SDL_Surface* screen;

/* Buttons */

Button::Button(Surface* image_, std::string info_, SDLKey binding_)
  : binding(binding_)
{
  image = image_;
  size = Vector(image->get_width(), image->get_height());
  id = 0;
  info = info_;
}

Button::~Button()
{
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
  if(pos.x + tanslation.x < 100 && pos.y + tanslation.y > SCREEN_HEIGHT - 20)
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

/* Group of buttons */

ButtonGroup::ButtonGroup(Vector pos_, Vector buttons_size_, Vector buttons_box_)
  : pos(pos_), buttons_size(buttons_size_), buttons_box(buttons_box_)
{
buttons.clear();
row = 0;
button_selected = -1;
mouse_hover = false;
mouse_left_button = false;
buttons_pair_nb = 0;
}

ButtonGroup::~ButtonGroup()
{
}

void ButtonGroup::add_button(Button button, int id, bool select)
{
button.pos.x = ((buttons.size()-buttons_pair_nb) % (int)buttons_box.x) * buttons_size.x;
button.pos.y = ((int)((buttons.size()-buttons_pair_nb) / buttons_box.x)) * buttons_size.y;
button.size = buttons_size;
button.id = id;
if(select)
  button_selected = id;

buttons.push_back(button);
}

void ButtonGroup::add_pair_of_buttons(Button button1, int id1, Button button2, int id2)
{
button1.pos.x = button2.pos.x = ((buttons.size()-buttons_pair_nb) % (int)buttons_box.x) * buttons_size.x;
button1.pos.y = button2.pos.y = ((int)((buttons.size()-buttons_pair_nb) / buttons_box.x)) * buttons_size.y;
button1.size.x = button2.size.x = buttons_size.x;
button1.size.y = button2.size.y = buttons_size.y / 2;
button2.pos.y += buttons_size.y / 2;
button1.id = id1;
button2.id = id2;

buttons_pair_nb++;
buttons.push_back(button1);
buttons.push_back(button2);
}

void ButtonGroup::draw(DrawingContext &context)
{
context.draw_filled_rect(pos - Vector(12,4),
        Vector(buttons_size.x*buttons_box.x + 16, buttons_size.y*buttons_box.y + 8),
        Color (0,0,0, 128), LAYER_GUI-1);

context.push_transform();
context.set_translation(Vector(-pos.x, -pos.y + buttons_size.y*row));
for(Buttons::iterator i = buttons.begin(); i != buttons.end(); ++i)
  {
  if(i->pos.y < row*buttons_size.y ||
      i->pos.y + i->size.y > (row + buttons_box.y) * buttons_size.y)
    continue;

  i->draw(context, i->id == button_selected);
  }
context.pop_transform();
}

bool ButtonGroup::event(SDL_Event &event)
{
bool caught_event = false;

switch(event.type)
  {
  case SDL_MOUSEMOTION:
    mouse_hover = false;

    if(mouse_left_button)
      {
      pos.x += int(event.motion.xrel * float(SCREEN_WIDTH)/screen->w);
      pos.y += int(event.motion.yrel * float(SCREEN_HEIGHT)/screen->h);
      caught_event = true;
      }
    if(event.button.x > pos.x-12 && event.button.x < pos.x+16 + buttons_box.x*buttons_size.x &&
       event.button.y > pos.y-4 && event.button.y < pos.y+8 + buttons_box.y*buttons_size.y)
      mouse_hover = true;
    break;
  case SDL_MOUSEBUTTONDOWN:
    if(event.button.x < pos.x-12 || event.button.x > pos.x+16 +
        buttons_box.x*buttons_size.x || event.button.y < pos.y-4 ||
        event.button.y > pos.y+8 + buttons_box.y*buttons_size.y)
      break;

    caught_event = true;

    if(event.button.button == SDL_BUTTON_WHEELUP)
      {
      row--;
      if(row < 0)
        row = 0;
      }
    else if(event.button.button == SDL_BUTTON_WHEELDOWN)
      {
      row++;
      if(row > (int)((buttons.size()-buttons_pair_nb)/buttons_box.x) - (int)buttons_box.y +
               ((int)(buttons.size()-buttons_pair_nb)%(int)buttons_box.x != 0 ? 1 : 0))
        row = (int)((buttons.size()-buttons_pair_nb)/buttons_box.x) - (int)buttons_box.y +
              ((int)(buttons.size()-buttons_pair_nb)%(int)buttons_box.x != 0 ? 1 : 0);
      }
    else if(event.button.button == SDL_BUTTON_LEFT)
      mouse_left_button = true;
    else
      caught_event = false;
    break;
  case SDL_MOUSEBUTTONUP:
    mouse_left_button = false;
    break;
  default:
    break;
  }

if(caught_event)
  return true;

for(Buttons::iterator i = buttons.begin(); i != buttons.end(); ++i)
  {
  if(i->pos.y < row*buttons_size.y ||
      i->pos.y + i->size.y > (row + buttons_box.y) * buttons_size.y)
    continue;

  if(i->event(event, (int)pos.x,
                     (int)pos.y - row*(int)buttons_size.y) == BT_SELECTED)
    {
    button_selected = i->id;
    caught_event = true;
    break;
    }
  }

return caught_event;
}

int ButtonGroup::selected_id()
{
return button_selected;
}

void ButtonGroup::set_unselected()
{
button_selected = -1;
}

bool ButtonGroup::is_hover()
{
return mouse_hover;
}

/* EOF */
