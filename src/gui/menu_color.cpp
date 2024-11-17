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

#include <regex>
#include <sstream>

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
  add_entry(MNID_COPY, _("Copy"));
  if (Color::s_clipboard_color != nullptr)
    add_entry(MNID_PASTE, _("Paste"), *Color::s_clipboard_color);
  else
    add_entry(MNID_PASTE, _("Paste"), Color(1.f, 1.f, 1.f));

  add_hl();
  add_entry(MNID_PASTE_CLIPBOARD, _("Paste clipboard"));

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
  else if (item.get_id() == MNID_PASTE_CLIPBOARD)
  {
    if (!SDL_HasClipboardText())
      return;

    const char* clipboard_text = SDL_GetClipboardText();
    if (!clipboard_text)
      return;

    const std::string text(clipboard_text);
    SDL_free(const_cast<char*>(clipboard_text));

    Color new_color;
    bool is_valid_format = false;

    // rgb(r,g,b)
    const std::regex rgb_format(R"(^\s*rgb\s*\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\)\s*$)");
    std::smatch rgb_matches;

    if (std::regex_match(text, rgb_matches, rgb_format))
    {
      const int r = std::stoi(rgb_matches[1].str());
      const int g = std::stoi(rgb_matches[2].str());
      const int b = std::stoi(rgb_matches[3].str());

      if (r >= 0 && r <= 255 && g >= 0 && g <= 255 && b >= 0 && b <= 255)
      {
        new_color = Color(static_cast<float>(r) / 255.0f, static_cast<float>(g) / 255.0f, static_cast<float>(b) / 255.0f, 1.0f);
        is_valid_format = true;
      }
    }
    else
    {
      // #rrggbb
      const std::regex hex_format(R"(^\s*#([A-Fa-f0-9]{6})\s*$)");
      std::smatch hex_matches;

      if (std::regex_match(text, hex_matches, hex_format))
      {
        const std::string hex_value = hex_matches[1].str();
        unsigned int hex_color;
        std::stringstream ss;
        ss << std::hex << hex_value;
        ss >> hex_color;

        const float r = ((hex_color >> 16) & 0xFF) / 255.0f;
        const float g = ((hex_color >> 8) & 0xFF) / 255.0f;
        const float b = (hex_color & 0xFF) / 255.0f;

        new_color = Color(r, g, b, 1.0f);
        is_valid_format = true;
      }
    }

    if (is_valid_format)
    {
      *color = new_color;

      Color::s_clipboard_color = std::make_unique<Color>(new_color);
      MenuItem& menu_paste_item = get_item_by_id(MNID_PASTE);
      menu_paste_item.set_text_color(new_color);
    }
    else
      log_warning << "Invalid color format: " << text << ". Supported formats: rgb(r,g,b) and #rrggbb" << std::endl;
  }
}

/* EOF */
