#ifndef __TILEMAP_H__
#define __TILEMAP_H__

#include "game_object.h"
#include "drawable.h"

class Level;

/**
 * This class is reponsible for drawing the level tiles
 */
class TileMap : public _GameObject, public Drawable
{
public:
  TileMap(DisplayManager& manager, Level* level);
  virtual ~TileMap();

  virtual void action(float elapsed_time);
  virtual void draw(ViewPort& viewport, int layer);
  virtual std::string type() const
  { return "TileMap"; }
  
private:
  Level* level;
};

#endif

