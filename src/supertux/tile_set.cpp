//  SuperTux
//  Copyright (C) 2008 Matthias Braun <matze@braunis.de>
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

#include "supertux/tile_set.hpp"

#include "editor/editor.hpp"
#include "supertux/resources.hpp"
#include "supertux/tile_set_parser.hpp"
#include "video/drawing_context.hpp"
#include "video/surface.hpp"

Tilegroup::Tilegroup() :
  name(),
  tiles()
{
  tiles.clear();
}

Tilegroup::~Tilegroup() {

}

/*
  tiles(),
  tiles_loaded(false),
  tilegroups()
{
  tiles.resize(1, 0);
  tiles[0] = new Tile();
  tilegroups.clear();
}

TileSet::TileSet(const std::string& filename) :
  tiles(),
  tiles_loaded(true),
  tilegroups()
*/
TileSet::TileSet() :
  m_tiles(1),
  notile_surface(Surface::create("images/tiles/auxiliary/notile.png")),
  tilegroups()
{
  m_tiles[0] = std::unique_ptr<Tile>(new Tile);
  tilegroups.clear();
}

TileSet::TileSet(const std::string& filename) :
  TileSet()
{
  TileSetParser parser(*this, filename);
  parser.parse();

  if (0)
  { // enable this if you want to see a list of free tiles
    log_info << "Last Tile ID is " << m_tiles.size()-1 << std::endl;
    int last = -1;
    for(int i = 0; i < int(m_tiles.size()); ++i)
    {
      if (m_tiles[i] == 0 && last == -1)
      {
        last = i;
      }
      else if (m_tiles[i] && last != -1)
      {
        log_info << "Free Tile IDs (" << i - last << "): " << last << " - " << i-1 << std::endl;
        last = -1;
      }
    }
  }

  if (0)
  { // enable this to dump the (large) list of tiles to log_debug
    // Two dumps are identical iff the tilesets specify identical tiles
    log_debug << "Tileset in " << filename << std::endl;
    for(int i = 0; i < int(m_tiles.size()); ++i)
    {
      if(m_tiles[i] != 0)
      {
        m_tiles[i]->print_debug(i);
      }
    }
  }
}

TileSet::~TileSet()
{
}

void
TileSet::add_tile(int id, std::unique_ptr<Tile> tile)
{
  if (id >= static_cast<int>(m_tiles.size())) {
    m_tiles.resize(id + 1);
  }

  if (m_tiles[id] != 0) {
    log_warning << "Tile with ID " << id << " redefined" << std::endl;
  } else {
    m_tiles[id] = std::move(tile);
  }
}

const Tile*
TileSet::get(const uint32_t id) const
{
  if (id >= m_tiles.size()) {
//    log_warning << "Invalid tile: " << id << std::endl;
    return m_tiles[0].get();
  } else {
    assert(id < m_tiles.size());
    Tile* tile = m_tiles[id].get();
    if(tile) {
      tile->load_images();
      return tile;
    } else {
//      log_warning << "Invalid tile: " << id << std::endl;
      return m_tiles[0].get();
    }
  }
}

void
TileSet::draw_tile(DrawingContext& context, uint32_t id, const Vector& pos,
                   int z_pos, Color color) const
{
  if (id == 0) return;
  Tile* tile;
  if (id >= m_tiles.size()) {
    tile = NULL;
  } else {
    tile = m_tiles[id].get();
  }

  if (tile) {
    tile->load_images();
    tile->draw(context, pos, z_pos, color);
  } else if (Editor::is_active()) { // Draw a notile sign
    context.draw_surface(notile_surface, pos, 0, color, Blend(), z_pos);
    context.draw_text(Resources::small_font, std::to_string(id),
                      pos + Vector(16, 16), ALIGN_CENTER, z_pos, color);
  }
}

/* EOF */
