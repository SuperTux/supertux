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
  color(color_),
  clipboard(ColorClipboard::instance())
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
  add_item(std::make_unique<MenuItem>(_("Copy"), 1));
  add_item(std::make_unique<MenuItem>(
    _("Paste"),
    2,
    clipboard.get_color() ? std::make_optional(*clipboard.get_color()) : std::nullopt));

  add_hl();
  add_back(_("OK"));
}

void
ColorMenu::menu_action(MenuItem& item)
{
  if (item.get_id() == 1)
  {
    clipboard.set_color(*color);
    MenuItem& menu_paste_item = get_item_by_id(2);
    menu_paste_item.set_text_color(*color);
  }
  else if (item.get_id() == 2)
  {
    const Color* clipboard_color = clipboard.get_color();
    if (clipboard_color)
      *color = *clipboard_color;
  }
}

/* EOF */
