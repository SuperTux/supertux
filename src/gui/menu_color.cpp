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

#include "audio/sound_manager.hpp"
#include "gui/menu_item.hpp"
#include "gui/item_action.hpp"
#include "util/gettext.hpp"
#include "video/color.hpp"

ColorMenu::ColorMenu(Color* color_) :
  color(color_)
{
  add_label(_("Mix the colour"));
  add_hl();

  add_colorchannel( &(color->red), Color(1.0f, 0.0f, 0.0f));
  add_colorchannel( &(color->green), Color(0.0f, 1.0f, 0.0f));
  add_colorchannel( &(color->blue), Color(0.0f, 0.0f, 1.0f));
  add_colorchannel( &(color->ultra_violet), Color(0.2f, 0.0f, 0.3f));
  add_colorchannel( &(color->alpha), Color(0.0f, 0.0f, 0.0f));
  add_colordisplay(color);

  add_hl();
  add_back(_("OK"));
}

void
ColorMenu::menu_action(MenuItem* item)
{

}

/* EOF */
