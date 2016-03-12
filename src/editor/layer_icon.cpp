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

#include "editor/layer_icon.hpp"

#include "math/rect.hpp"
#include "object/background.hpp"
#include "object/gradient.hpp"
#include "object/tilemap.hpp"
#include "supertux/colorscheme.hpp"
#include "supertux/game_object.hpp"
#include "supertux/globals.hpp"
#include "supertux/resources.hpp"
#include "video/renderer.hpp"
#include "video/video_system.hpp"

LayerIcon::LayerIcon(std::string icon, GameObject *layer_) :
  ObjectIcon("", icon),
  layer(layer_)
{
}

LayerIcon::~LayerIcon() {

}

void
LayerIcon::draw(DrawingContext& context, Vector pos) {
  ObjectIcon::draw(context,pos);
  int l = get_zpos();
  if (l != -9999) {
    context.draw_text(Resources::small_font, std::to_string(l),
                      pos + Vector(16,16),
                      ALIGN_CENTER, LAYER_GUI, ColorScheme::Menu::default_color);
  }
}

int
LayerIcon::get_zpos() {
  if (layer->get_class() == "tilemap") {
    return ((TileMap*)layer)->get_layer();
  }
  if (layer->get_class() == "background") {
    return ((Background*)layer)->get_layer();
  }
  if (layer->get_class() == "gradient") {
    return ((Gradient*)layer)->get_layer();
  }
  return -9999;
}
