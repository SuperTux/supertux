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

#include "object/background.hpp"
#include "object/gradient.hpp"
#include "object/particlesystem.hpp"
#include "object/particlesystem_interactive.hpp"
#include "object/tilemap.hpp"
#include "supertux/colorscheme.hpp"
#include "supertux/game_object.hpp"
#include "supertux/resources.hpp"
#include "video/surface.hpp"

LayerIcon::LayerIcon(GameObject* layer) :
  ObjectIcon("", layer->get_icon_path()),
  m_layer(layer),
  m_selection()
{
  if (dynamic_cast<TileMap*>(layer)) {
    m_selection = Surface::from_file("images/engine/editor/selection.png");
  }
}

void
LayerIcon::draw(DrawingContext& context, const Vector& pos)
{
  if (!is_valid()) return;

  ObjectIcon::draw(context, pos);
  int l = get_zpos();
  if (l != std::numeric_limits<int>::min()) {
    context.color().draw_text(Resources::small_font, std::to_string(l),
                                pos + Vector(16,16),
                                ALIGN_CENTER, LAYER_GUI, ColorScheme::Menu::default_color);
    if (TileMap* tilemap = dynamic_cast<TileMap*>(m_layer)) {
      if (tilemap->m_editor_active) {
        context.color().draw_surface(m_selection, pos, LAYER_GUI - 1);
      }
    }
  }
}

void
LayerIcon::draw(DrawingContext& context, const Vector& pos, int pixels_shown)
{
  if (!is_valid()) return;

  ObjectIcon::draw(context, pos, pixels_shown);
  int l = get_zpos();
  if (l != std::numeric_limits<int>::min()) {
    // Don't draw the text if the icon is not 100% visible
    if (TileMap* tilemap = dynamic_cast<TileMap*>(m_layer)) {
      if (tilemap->m_editor_active) {
        context.color().draw_surface(m_selection, pos, LAYER_GUI - 1);
      }
    }
  }
}

int
LayerIcon::get_zpos() const
{
  if (!is_valid()) {
    return std::numeric_limits<int>::min();
  }

  if (auto* tilemap = dynamic_cast<TileMap*>(m_layer)) {
    return tilemap->get_layer();
  } else if (auto* bkgrd = dynamic_cast<Background*>(m_layer)) {
    return bkgrd->get_layer();
  } else if (auto* grd = dynamic_cast<Gradient*>(m_layer)) {
    return grd->get_layer();
  } else if (auto* ps = dynamic_cast<ParticleSystem*>(m_layer)) {
    return ps->get_layer();
  } else if (auto* psi = dynamic_cast<ParticleSystem_Interactive*>(m_layer)) {
    return psi->get_layer();
  } else {
    return std::numeric_limits<int>::min();
  }
}

bool
LayerIcon::is_tilemap() const
{
  return dynamic_cast<TileMap*>(m_layer) != nullptr;
}

bool
LayerIcon::is_valid() const
{
  return m_layer && m_layer->is_valid();
}

/* EOF */
