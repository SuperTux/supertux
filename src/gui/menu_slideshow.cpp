//  SuperTux
//  Copyright (C) 2018 christ2go <christian@hagemeier.ch>
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

#include "gui/menu.hpp"
#include "gui/menu_manager.hpp"
#include "gui/menu_slideshow.hpp"
#include "math/vector.hpp"
#include "supertux/colorscheme.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/resources.hpp"
#include "supertux/timer.hpp"
#include "video/color.hpp"
#include "video/drawing_context.hpp"
#include "video/font.hpp"
#include "video/renderer.hpp"
#include "video/video_system.hpp"

void Slideshow::draw(DrawingContext &context, const Vector & pos, int menu_width,
                     bool active) {
  if (m_images.size() == 0)
    return;
  int index = int(real_time / 3) % m_images.size();
  std::string caption = m_text[index];
  int opx = pos.x;
  context.draw_surface(
      m_images[index],
      Vector(pos.x + menu_width / 2 - m_images[index]->get_width() / 2, pos.y),
      LAYER_GUI);
  context.draw_text(
      Resources::small_font, caption,
      Vector(opx + menu_width / 2, pos.y + m_images[index]->get_height() + 3),
      ALIGN_CENTER, LAYER_GUI,
      active ? ColorScheme::Menu::active_color : get_color());
}
