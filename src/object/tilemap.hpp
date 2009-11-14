//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef SUPERTUX_TILEMAP_H
#define SUPERTUX_TILEMAP_H

#include <vector>
#include <stdint.h>
#include <string>

#include "game_object.hpp"
#include "serializable.hpp"
#include "math/vector.hpp"
#include "video/drawing_context.hpp"
#include "object/path.hpp"
#include "object/path_walker.hpp"
#include "script_interface.hpp"

namespace lisp {
class Lisp;
}

class Level;
class TileManager;
class Tile;
class TileSet;

/**
 * This class is responsible for drawing the level tiles
 */
class TileMap : public GameObject, public Serializable, public ScriptInterface
{
public:
  TileMap(const TileSet *tileset);
  TileMap(const lisp::Lisp& reader);
  TileMap(const TileSet *tileset, std::string name, int z_pos, bool solid_,
          size_t width_, size_t height_);
  virtual ~TileMap();

  virtual void write(lisp::Writer& writer);

  virtual void update(float elapsed_time);
  virtual void draw(DrawingContext& context);

  /** Move tilemap until at given node, then stop */
  void goto_node(int node_no);

  /** Start moving tilemap */
  void start_moving();

  /** Stop tilemap at next node */
  void stop_moving();

  virtual void expose(HSQUIRRELVM vm, SQInteger table_idx);
  virtual void unexpose(HSQUIRRELVM vm, SQInteger table_idx);

  void set(int width, int height, const std::vector<unsigned int>& vec,
      int z_pos, bool solid);

  /** resizes the tilemap to a new width and height (tries to not destroy the
   * existing map)
   */
  void resize(int newwidth, int newheight, int fill_id = 0);

  size_t get_width() const
  { return width; }

  size_t get_height() const
  { return height; }

  float get_x_offset() const
  { return x_offset; }

  float get_y_offset() const
  { return y_offset; }

  const Vector& get_movement() const
  {
    return movement;
  }

  void set_x_offset(float x_offset)
  { this->x_offset = x_offset; }

  void set_y_offset(float y_offset)
  { this->y_offset = y_offset; }

  int get_layer() const
  { return z_pos; }

  bool is_solid() const
  { return solid; }

  /**
   * Changes Tilemap's solidity, i.e. whether to consider it when doing collision detection.
   */
  void set_solid(bool solid = true);

  /// returns tile in row y and column y (of the tilemap)
  const Tile* get_tile(int x, int y) const;
  /// returns tile at position pos (in world coordinates)
  const Tile* get_tile_at(const Vector& pos) const;
  /// returns tile in row y and column y (of the tilemap)
  uint32_t get_tile_id(int x, int y) const;
  /// returns tile at position pos (in world coordinates)
  uint32_t get_tile_id_at(const Vector& pos) const;

  void change(int x, int y, uint32_t newtile);

  void change_at(const Vector& pos, uint32_t newtile);

  /// changes all tiles with the given ID
  void change_all(uint32_t oldtile, uint32_t newtile);

  void set_drawing_effect(DrawingEffect effect)
  {
    drawing_effect = effect;
  }

  DrawingEffect get_drawing_effect()
  {
    return drawing_effect;
  }

  /**
   * Start fading the tilemap to opacity given by @c alpha.
   * Destination opacity will be reached after @c seconds seconds. Also influences solidity.
   */
  void fade(float alpha, float seconds = 0);

  /**
   * Instantly switch tilemap's opacity to @c alpha. Also influences solidity.
   */
  void set_alpha(float alpha);

  /**
   * Return tilemap's opacity. Note that while the tilemap is fading in or out, this will return the current alpha value, not the target alpha.
   */
  float get_alpha();

private:
  const TileSet *tileset;

  typedef std::vector<uint32_t> Tiles;
  Tiles tiles;

  bool solid;
  float speed_x;
  float speed_y;
  int width, height;
  int z_pos;
  float x_offset;
  float y_offset;
  Vector movement; /**< The movement that happened last frame */

  DrawingEffect drawing_effect;
  float alpha; /**< requested tilemap opacity */
  float current_alpha; /**< current tilemap opacity */
  float remaining_fade_time; /**< seconds until requested tilemap opacity is reached */

  std::auto_ptr<Path> path;
  std::auto_ptr<PathWalker> walker;

  DrawingContext::Target draw_target; /**< set to LIGHTMAP to draw to lightmap */
};

#endif /*SUPERTUX_TILEMAP_H*/
