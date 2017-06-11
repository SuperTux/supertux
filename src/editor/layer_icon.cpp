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

#include <limits>

#include "math/rect.hpp"
#include "object/background.hpp"
#include "object/gradient.hpp"
#include "object/particlesystem.hpp"
#include "object/particlesystem_interactive.hpp"
#include "object/tilemap.hpp"
#include "supertux/colorscheme.hpp"
#include "supertux/game_object.hpp"
#include "supertux/globals.hpp"
#include "supertux/resources.hpp"
#include "video/renderer.hpp"
#include "video/video_system.hpp"

LayerIcon::LayerIcon(GameObject *layer_) :
  ObjectIcon("", layer_->get_icon_path()),
  layer(layer_),
  is_tilemap(false),
  selection()
{
  auto tm = dynamic_cast<TileMap*>(layer_);
  if (tm) {
    is_tilemap = true;
    selection = Surface::create("images/engine/editor/selection.png");
  }
}

LayerIcon::~LayerIcon() {

}

void
LayerIcon::draw(DrawingContext& context, const Vector& pos) {
  if (!is_valid()) return;

  ObjectIcon::draw(context,pos);
  int l = get_zpos();
  if (l != std::numeric_limits<int>::min()) {
    context.draw_text(Resources::small_font, std::to_string(l),
                      pos + Vector(16,16),
                      ALIGN_CENTER, LAYER_GUI, ColorScheme::Menu::default_color);
    if (is_tilemap) if (((TileMap*)layer)->editor_active) {
      context.draw_surface(selection, pos, LAYER_GUI - 1);
    }
  }
}

int
LayerIcon::get_zpos() const {
  if(!is_valid()) {
    return std::numeric_limits<int>::min();
  }

  if (is_tilemap) { //When the layer is a tilemap, the class is obvious.
    return ((TileMap*)layer)->get_layer();
  }

  auto bkgrd = dynamic_cast<Background*>(layer);
  if (bkgrd) {
    return bkgrd->get_layer();
  }

  auto grd = dynamic_cast<Gradient*>(layer);
  if (grd) {
    return grd->get_layer();
  }

  auto ps = dynamic_cast<ParticleSystem*>(layer);
  if (ps) {
    return ps->get_layer();
  }

  auto psi = dynamic_cast<ParticleSystem_Interactive*>(layer);
  if (psi) {
    return psi->get_layer();
  }

  return std::numeric_limits<int>::min();
}

bool
LayerIcon::is_valid() const {
  return layer && layer->is_valid();
}
/* EOF */
