//
// C++ Interface: tile
//
// Description: 
//
//
// Author: Tobias Glaesser <tobi.web@gmx.de>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef TILE_H
#define TILE_H

#include <vector>
#include "texture.h"
#include "globals.h"
#include "lispreader.h"
#include "setup.h"

/**
Tile Class
*/
struct Tile
{
std::vector<texture_type> images;
bool solid;
bool brick;
bool ice;
bool fullbox;
int anim_speed;
unsigned char alpha;
};

class TileManager
{
private:
 TileManager();
 std::vector<Tile*> tiles;
 static TileManager* instance_ ;
 void load_tileset(std::string filename);
  
public:
 static TileManager* instance() { return instance_ ? instance_ : instance_ = new TileManager(); }
 Tile* get(unsigned int id) { if( id < tiles.size()) { return tiles[id]; } else { return NULL; } } ;

};

#endif
