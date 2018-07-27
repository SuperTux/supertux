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

#include "editor/tip.hpp"

#include "supertux/colorscheme.hpp"
#include "supertux/game_object.hpp"
#include "supertux/resources.hpp"
#include "util/log.hpp"
#include "video/drawing_context.hpp"

Tip::Tip(GameObject* object) :
  strings(),
  header()
{
  if (!object) {
    log_warning << "Editor/Tip: Given object doesn't exist." << std::endl;
    return;
  }

  auto os = object->get_settings();
  header = os.name;

  for(const auto& oo : os.options) {
    if (oo.type != MN_REMOVE && (oo.flags & OPTION_VISIBLE)) {
      auto value = oo.to_string();
      if(value.empty()) {
        value = _("<empty>");
      }
      strings.push_back(oo.text + ": " + value);
    }
  }
}

void
Tip::draw(DrawingContext& context, const Vector& pos) {
  auto position = pos;
  position.y += 35;
  context.color().draw_text(Resources::normal_font, header, position,
                              ALIGN_LEFT, LAYER_GUI-11, ColorScheme::Menu::label_color);

  for(const auto& str : strings) {
    position.y += 22;
    context.color().draw_text(Resources::normal_font, str, position,
                                ALIGN_LEFT, LAYER_GUI-11, ColorScheme::Menu::default_color);
  }
}

void
Tip::draw_up(DrawingContext& context, const Vector& pos) {
  auto position = Vector(pos.x, pos.y - (strings.size() + 1) * 22 - 35);
  draw(context, position);
}

/* EOF */
