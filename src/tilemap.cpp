#include "tilemap.h"
#include "display_manager.h"
#include "level.h"
#include "tile.h"
#include "globals.h"

TileMap::TileMap(DisplayManager& display_manager, Level* newlevel)
  : level(newlevel)
{
  display_manager.add_drawable(this, LAYER_BACKGROUNDTILES);
  display_manager.add_drawable(this, LAYER_TILES);
  display_manager.add_drawable(this, LAYER_FOREGROUNDTILES);
}

TileMap::~TileMap()
{
}

void
TileMap::action(float )
{
}

void
TileMap::draw(ViewPort& viewport, int layer)
{
  std::vector<std::vector<unsigned int> >* tiles;
  switch(layer) {
    case LAYER_BACKGROUNDTILES:
      tiles = &level->bg_tiles; break;
    case LAYER_TILES:
      tiles = &level->ia_tiles; break;
    case LAYER_FOREGROUNDTILES:
      tiles = &level->fg_tiles; break;
    default:
      assert(!"Wrong layer when drawing tilemap.");
  }

  int tsx = int(viewport.get_translation().x / 32); // tilestartindex x
  int tsy = int(viewport.get_translation().y / 32); // tilestartindex y
  int sx = - (int(viewport.get_translation().x) % 32);
  int sy = - (int(viewport.get_translation().y) % 32);
  for(int x = sx, tx = tsx; x < screen->w && tx < int((*tiles)[0].size());
      x += 32, ++tx) {
    for(int y = sy, ty = tsy; y < screen->h && ty < int(tiles->size());
          y += 32, ++ty) {
      Tile::draw(x, y, (*tiles) [ty][tx]);
    }
  }
}
