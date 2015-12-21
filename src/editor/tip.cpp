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
#include "util/gettext.hpp"
#include "video/color.hpp"
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
  std::string text;

  for(auto i = os.options.begin(); i != os.options.end(); ++i) {
    ObjectOption* oo = &(*i);
    text = oo->text + ": ";
    switch (oo->type) {
      case MN_TEXTFIELD:
        text += *((std::string*)(oo->option));
        break;
      case MN_NUMFIELD:
        text += std::to_string(*((float*)(oo->option)));
        break;
      case MN_INTFIELD:
        text += std::to_string(*((int*)(oo->option)));
        break;
      case MN_TOGGLE:
        text += (*((bool*)(oo->option))) ? _("true") : _("false");
        break;
      case MN_STRINGSELECT:
        text += oo->select[*((int*)(oo->option))];
        break;
      case MN_COLOR:
        text += std::to_string(((Color*)oo->option)->red) + " ";
        text += std::to_string(((Color*)oo->option)->green) + " ";
        text += std::to_string(((Color*)oo->option)->blue);
        break;
      default:
        text += _("Unknown");
        break;
    }
    strings.push_back(text);
  }
}

Tip::~Tip() {

}

void
Tip::draw(DrawingContext& context, Vector pos) {
  context.draw_text(Resources::normal_font, header, pos,
                    ALIGN_RIGHT, LAYER_GUI-11, ColorScheme::Menu::label_color);

  for(auto i = strings.begin(); i != strings.end(); ++i) {
    std::string* str = &(*i);
    pos.y += 22;
    context.draw_text(Resources::normal_font, *str, pos,
                      ALIGN_RIGHT, LAYER_GUI-11, ColorScheme::Menu::default_color);
  }
}

void
Tip::draw_up(DrawingContext& context, Vector pos) {
  draw(context, Vector(pos.x, pos.y - (strings.size()+1)*22 ));
}

/* EOF */
