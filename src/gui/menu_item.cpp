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

#include "gui/menu_item.hpp"

#include "supertux/resources.hpp"
#include "supertux/timer.hpp"
#include "video/font.hpp"

static const float FLICK_CURSOR_TIME   = 0.5f;

MenuItem::MenuItem(MenuItemKind _kind, int _id) :
  kind(_kind),
  id(_id),
  toggled(),
  text(),
  input(),
  help(),
  list(),
  selected(),
  target_menu(),
  input_flickering()
{
  toggled = false;
  selected = false;
  target_menu = 0;
}

void
MenuItem::change_text(const  std::string& text_)
{
  text = text_;
}

void
MenuItem::change_input(const  std::string& text_)
{
  input = text_;
}

void
MenuItem::set_help(const std::string& help_text)
{
  std::string overflow;
  help = Resources::normal_font->wrap_to_width(help_text, 600, &overflow);
  while (!overflow.empty())
  {
    help += "\n";
    help += Resources::normal_font->wrap_to_width(overflow, 600, &overflow);
  }
}

std::string
MenuItem::get_input_with_symbol(bool active_item)
{
  if(!active_item) {
    input_flickering = true;
  } else {
    input_flickering = ((int) (real_time / FLICK_CURSOR_TIME)) % 2;
  }

  char str[1024];
  if(input_flickering)
    snprintf(str, sizeof(str), "%s ",input.c_str());
  else
    snprintf(str, sizeof(str), "%s_",input.c_str());

  std::string string = str;

  return string;
}

/* EOF */
