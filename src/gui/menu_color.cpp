//  SuperTux
//  Copyright (C) 2015 Hume2 <teratux.mail@gmail.com>
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

#include "gui/menu_color.hpp"
#include "menu_item.hpp"

#include "util/gettext.hpp"

ColorMenu::ColorMenu(Color* color_) :
  color(color_)
{
  add_label(_("Mix the colour"));
  add_hl();

  add_color_picker_2d(*color);
  add_color_channel_rgba(&(color->red), Color::RED);
  add_color_channel_rgba(&(color->green), Color::GREEN);
  add_color_channel_rgba(&(color->blue), Color::BLUE);
  add_color_channel_rgba(&(color->alpha), Color::BLACK, -1, true);
  add_color_display(color);

  add_hl();
  add_item(std::make_unique<MenuItem>(_("Copy"), MNID_COPY));
  add_item(std::make_unique<MenuItem>(
    _("Paste"),
    MNID_PASTE,
    Color::s_clipboard_color ? std::make_optional(*Color::s_clipboard_color) : std::nullopt));

  add_hl();
  add_back(_("OK"));
}

void
ColorMenu::menu_action(MenuItem& item)
{
  if (item.get_id() == MNID_COPY)
  {
    if (color)
    {
      Color::s_clipboard_color = std::make_unique<Color>(*color);
      MenuItem& menu_paste_item = get_item_by_id(MNID_PASTE);
      menu_paste_item.set_text_color(*color);
    }
  }
  else if (item.get_id() == MNID_PASTE)
  {
    if (Color::s_clipboard_color)
      *color = *Color::s_clipboard_color;
  }
}

/* EOF */
