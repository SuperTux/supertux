//  SuperTux
//  Copyright (C) 2003 Tobias Glaesser <tobi.web@gmx.de>
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

#include "supertux/resources.hpp"

#include <physfs.h>

#include "gui/mousecursor.hpp"
#include "util/gettext.hpp"
#include "util/log.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"
#include "video/font.hpp"
#include "video/surface.hpp"

std::unique_ptr<MouseCursor> Resources::mouse_cursor;

FontPtr Resources::fixed_font;
FontPtr Resources::normal_font;
FontPtr Resources::small_font;
FontPtr Resources::big_font;

FontPtr Resources::fixed_font_fallback;
FontPtr Resources::normal_font_fallback;
FontPtr Resources::small_font_fallback;
FontPtr Resources::big_font_fallback;

SurfacePtr Resources::checkbox;
SurfacePtr Resources::checkbox_checked;
SurfacePtr Resources::back;
SurfacePtr Resources::arrow_left;
SurfacePtr Resources::arrow_right;
SurfacePtr Resources::no_tile;

Resources::Resources()
{
  // Load the mouse-cursor
  mouse_cursor.reset(new MouseCursor("images/engine/menu/mousecursor.png",
                                     "images/engine/menu/mousecursor-click.png",
                                     "images/engine/menu/mousecursor-link.png"));
  MouseCursor::set_current(mouse_cursor.get());

  fixed_font_fallback.reset(new Font("fonts/ttf/FreeMono.ttf", 18));
  normal_font_fallback.reset(new Font("fonts/ttf/FreeSansBold.ttf", 18));
  small_font_fallback.reset(new Font("fonts/ttf/FreeSans.ttf", 10, 1));
  big_font_fallback.reset(new Font("fonts/ttf/FreeSansBold.ttf", 25, 3));

  reload_font_config();

  /* Load menu images */
  checkbox = Surface::from_file("images/engine/menu/checkbox-unchecked.png");
  checkbox_checked = Surface::from_file("images/engine/menu/checkbox-checked.png");
  back = Surface::from_file("images/engine/menu/arrow-back.png");
  arrow_left = Surface::from_file("images/engine/menu/arrow-left.png");
  arrow_right = Surface::from_file("images/engine/menu/arrow-right.png");
  no_tile = Surface::from_file("images/tiles/auxiliary/notile.png");
}

void
Resources::reload_font_config()
{
  auto font_config_filename = "fonts/fonts.txt";
  auto current_locale = g_dictionary_manager->get_language().get_language();
  if(!PHYSFS_exists(font_config_filename))
  {
    log_warning << "Couldn't find font config file. Using defaults." << std::endl;
    use_fallback_fonts();
    return;
  }

  auto doc = ReaderDocument::parse(font_config_filename);
  auto root = doc.get_root();
  if(root.get_name() != "font-config")
  {
    use_fallback_fonts();
  }
  else
  {
    bool found_item = false;
    auto fontconfig = root.get_mapping();
    auto iter = fontconfig.get_iter();
    while(iter.next()) // Iterator for locales
    {
      auto current = iter.as_mapping();
      std::string lang = "";
      current.get("lang", lang);
      if(lang != current_locale)
      {
        continue;
      }

      found_item = true;

      auto fonts_iter = current.get_iter();
      while(fonts_iter.next())
      {
        if(fonts_iter.get_key() != "font")
        {
          continue;
        }

        auto font_map = fonts_iter.as_mapping();

        std::string style = "";
        std::string path = "";
        int font_size = 0;

        font_map.get("style", style, "normal");
        font_map.get("path", path);
        font_map.get("size", font_size, 10);

        auto font = new Font(path, font_size);

        if(style == "fixed")
        {
          fixed_font.reset(font);
        }
        if(style == "small")
        {
          small_font.reset(font);
        }
        if(style == "normal")
        {
          normal_font.reset(font);
        }
        if(style == "big")
        {
          big_font.reset(font);
        }
      }
    }
    
    if(!found_item)
    {
      use_fallback_fonts();
    }
  }
}

void
Resources::use_fallback_fonts()
{
  fixed_font.reset(fixed_font_fallback.get());
  normal_font.reset(normal_font_fallback.get());
  small_font.reset(small_font_fallback.get());
  big_font.reset(big_font_fallback.get());
}

Resources::~Resources()
{
  // Free menu images
  no_tile.reset();
  checkbox.reset();
  checkbox_checked.reset();
  back.reset();
  arrow_left.reset();
  arrow_right.reset();

  // Free global images:
  fixed_font.reset();
  normal_font.reset();
  small_font.reset();
  big_font.reset();

  fixed_font_fallback.reset();
  normal_font_fallback.reset();
  small_font_fallback.reset();
  big_font_fallback.reset();

  mouse_cursor.reset();
}

/* EOF */
