//  SuperTux
//  Copyright (C) 2015 Hume2 <teratux.mail@gmail.com>
//                2024 bruhmoent
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
#include "../util/log.hpp"

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
  add_entry(MNID_COPY_CLIPBOARD, _("Copy from clipboard"));
  if (Color::s_clipboard_color != nullptr)
    add_entry(MNID_PASTE_CLIPBOARD, _("Paste from clipboard"), *Color::s_clipboard_color);
  else
    add_entry(MNID_PASTE_CLIPBOARD, _("Paste from clipboard"), Color(1.f, 1.f, 1.f));

  add_hl();
  add_back(_("OK"));
}

void
ColorMenu::menu_action(MenuItem& item)
{
  if (item.get_id() == MNID_COPY_CLIPBOARD)
  {
    if (color)
    {
      Color::s_clipboard_color = std::make_unique<Color>(*color);
      MenuItem& menu_paste_item = get_item_by_id(MNID_PASTE_CLIPBOARD);
      menu_paste_item.set_text_color(*color);

      std::stringstream ss;
      ss << "rgb("
         << static_cast<int>(color->red * 255.f) << ","
         << static_cast<int>(color->green * 255.f) << ","
         << static_cast<int>(color->blue * 255.f) << ")";

      const std::string clipboard_text = ss.str();

      if (SDL_SetClipboardText(clipboard_text.c_str()) != 0)
        log_warning << "Failed to set SDL clipboard text: " << SDL_GetError() << std::endl;
    }
  }
  else if (item.get_id() == MNID_PASTE_CLIPBOARD)
  {
    if (SDL_HasClipboardText())
    {
      const char* clipboard_text = SDL_GetClipboardText();
      if (!clipboard_text)
        return;

      const std::string text(clipboard_text);
      SDL_free(const_cast<char*>(clipboard_text));

      std::optional<Color> new_color;
      new_color = Color::from_rgb_string(text);

      if (!new_color)
        new_color = Color::from_hex_string(text);

      if (new_color)
      {
        *color = *new_color;
        Color::s_clipboard_color = std::make_unique<Color>(*new_color);
        MenuItem& menu_paste_item = get_item_by_id(MNID_PASTE_CLIPBOARD);
        menu_paste_item.set_text_color(*new_color);
      }
      else
        log_warning << "Invalid color format: " << text << ". Supported formats: rgb(r,g,b) and #rrggbb" << std::endl;
    }
  }
}

/* EOF */
