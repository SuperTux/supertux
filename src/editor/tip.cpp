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

#include "editor/object_settings.hpp"
#include "supertux/colorscheme.hpp"
#include "supertux/game_object.hpp"
#include "supertux/globals.hpp"
#include "supertux/resources.hpp"
#include "video/drawing_context.hpp"
#include "video/font.hpp"
#include "video/renderer.hpp"
#include "video/video_system.hpp"

Tip::Tip(GameObject* object) :
  strings(),
  header("")
{
  strings.clear();
  if (!object) {
    log_warning << "Editor/Tip: Given object doesn't exist." << std::endl;
    return;
  }

  ObjectSettings os = object->get_settings();
  header = os.name;

  for(auto& oo : os.options) {
    strings.push_back(oo.text + ": " + oo.to_string());
  }
}

Tip::~Tip() {

}

void
Tip::draw(DrawingContext& context, Vector pos) {
  pos.y += 35;
  context.draw_text(Resources::normal_font, header, pos,
                    ALIGN_LEFT, LAYER_GUI-11, ColorScheme::Menu::label_color);

  for(auto& str : strings) {
    pos.y += 22;
    context.draw_text(Resources::normal_font, str, pos,
                      ALIGN_LEFT, LAYER_GUI-11, ColorScheme::Menu::default_color);
  }
}

void
Tip::draw_up(DrawingContext& context, Vector pos) {
  draw(context, Vector(pos.x, pos.y - (strings.size() + 1) * 22 ));
}

/* EOF */
