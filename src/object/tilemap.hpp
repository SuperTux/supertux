//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#ifndef HEADER_SUPERTUX_OBJECT_TILEMAP_HPP
#define HEADER_SUPERTUX_OBJECT_TILEMAP_HPP

#include <boost/shared_ptr.hpp>

#include "object/path_walker.hpp"
#include "supertux/game_object.hpp"
#include "supertux/script_interface.hpp"
#include "video/drawing_context.hpp"

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
class TileMap : public GameObject,
                public ScriptInterface
{
public:
  TileMap(const TileSet *tileset);
  TileMap(const Reader& reader);
  TileMap(const TileSet *tileset, std::string name, int z_pos, bool solid_,
          size_t width_, size_t height_);
  virtual ~TileMap();

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

  Vector get_offset() const
  { return offset; }

  /** Get the movement of this tilemap. The collision detection code may need a
   *  non-negative y-movement. Passing `false' as the `actual' argument will
   *  provide that. Used exclusively in src/supertux/sector.cpp. */
  Vector get_movement(bool actual) const
  {
    if(actual) {
      return movement;
    } else {
      return Vector(movement.x, std::max(0.0f,movement.y));
    }
  }

  boost::shared_ptr<Path> get_path()
  { return path; }

  boost::shared_ptr<PathWalker> get_walker()
  { return walker; }

  void set_offset(const Vector &offset_)
  { this->offset = offset_; }

  /* Returns the position of the upper-left corner of
   * tile (x, y) in the sector. */
  Vector get_tile_position(int x, int y) const
  { return offset + Vector(x,y) * 32; }

  Rectf get_bbox() const
  { return Rectf(get_tile_position(0, 0), get_tile_position(width, height)); }

  Rectf get_tile_bbox(int x, int y) const
  { return Rectf(get_tile_position(x, y), get_tile_position(x+1, y+1)); }

  /* Returns the half-open rectangle of (x, y) tile indices
   * that overlap the given rectangle in the sector. */
  Rect get_tiles_overlapping(const Rectf &rect) const;

  int get_layer() const
  { return z_pos; }

  bool is_solid() const
  { return real_solid && effective_solid; }

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

  /* read solid: In *general*, is this a solid layer?
   * effective solid: is the layer *currently* solid? A generally solid layer
   * may be not solid when its alpha is low.
   * See `is_solid' above. */
  bool real_solid;
  bool effective_solid;
  void update_effective_solid (void);

  float speed_x;
  float speed_y;
  int width, height;
  int z_pos;
  Vector offset;
  Vector movement; /**< The movement that happened last frame */

  DrawingEffect drawing_effect;
  float alpha; /**< requested tilemap opacity */
  float current_alpha; /**< current tilemap opacity */
  float remaining_fade_time; /**< seconds until requested tilemap opacity is reached */

  boost::shared_ptr<Path> path;
  boost::shared_ptr<PathWalker> walker;

  DrawingContext::Target draw_target; /**< set to LIGHTMAP to draw to lightmap */

private:
  TileMap(const TileMap&);
  TileMap& operator=(const TileMap&);
};

#endif /*SUPERTUX_TILEMAP_H*/

/* EOF */
