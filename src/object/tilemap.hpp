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

#include <algorithm>

#include "math/rect.hpp"
#include "math/rectf.hpp"
#include "math/size.hpp"
#include "object/path_object.hpp"
#include "object/path_walker.hpp"
#include "squirrel/exposed_object.hpp"
#include "scripting/tilemap.hpp"
#include "supertux/autotile.hpp"
#include "supertux/game_object.hpp"
#include "video/color.hpp"
#include "video/flip.hpp"
#include "video/drawing_target.hpp"

class DrawingContext;
class Tile;
class TileSet;

/** This class is responsible for drawing the level tiles */
class TileMap final :
  public GameObject,
  public ExposedObject<TileMap, scripting::TileMap>,
  public PathObject
{
public:
  TileMap(const TileSet *tileset);
  TileMap(const TileSet *tileset, const ReaderMapping& reader);
  virtual ~TileMap();

  virtual void finish_construction() override;

  virtual std::string get_class() const override { return "tilemap"; }
  virtual const std::string get_icon_path() const override { return "images/engine/editor/tilemap.png"; }
  virtual std::string get_display_name() const override { return _("Tilemap"); }

  virtual ObjectSettings get_settings() override;
  virtual void after_editor_set() override;

  virtual void update(float dt_sec) override;
  virtual void draw(DrawingContext& context) override;

  virtual void editor_update() override;

  /** Move tilemap until at given node, then stop */
  void goto_node(int node_no);

  /** Start moving tilemap */
  void start_moving();

  /** Stop tilemap at next node */
  void stop_moving();

  void set(int width, int height, const std::vector<unsigned int>& vec,
           int z_pos, bool solid);

  /** resizes the tilemap to a new width and height (tries to not
      destroy the existing map) */
  void resize(int newwidth, int newheight, int fill_id = 0,
              int xoffset = 0, int yoffset = 0);
  void resize(const Size& newsize, const Size& resize_offset);

  int get_width() const { return m_width; }
  int get_height() const { return m_height; }
  Size get_size() const { return Size(m_width, m_height); }

  void set_offset(const Vector &offset_) { m_offset = offset_; }
  Vector get_offset() const { return m_offset; }

  void move_by(const Vector& pos);

  /** Get the movement of this tilemap. The collision detection code
      may need a non-negative y-movement. Passing `false' as the
      `actual' argument will provide that. Used exclusively in
      src/supertux/sector.cpp. */
  Vector get_movement(bool actual) const
  {
    if (actual) {
      return m_movement;
    } else {
      return Vector(m_movement.x, std::max(0.0f, m_movement.y));
    }
  }

  /** Returns the position of the upper-left corner of tile (x, y) in
      the sector. */
  Vector get_tile_position(int x, int y) const
  { return m_offset + Vector(static_cast<float>(x), static_cast<float>(y)) * 32.0f; }

  Rectf get_bbox() const {
    return Rectf(get_tile_position(0, 0),
                 get_tile_position(m_width, m_height));
  }

  Rectf get_tile_bbox(int x, int y) const {
    return Rectf(get_tile_position(x, y),
                 get_tile_position(x + 1, y + 1));
  }

  /** Returns the half-open rectangle of (x, y) tile indices that
      overlap the given rectangle in the sector. */
  Rect get_tiles_overlapping(const Rectf &rect) const;

  int get_layer() const { return m_z_pos; }
  void set_layer(int layer_) { m_z_pos = layer_; }

  bool is_solid() const { return m_real_solid && m_effective_solid; }

  /** Changes Tilemap's solidity, i.e. whether to consider it when
      doing collision detection. */
  void set_solid(bool solid = true);

  const Tile& get_tile(int x, int y) const;
  const Tile& get_tile_at(const Vector& pos) const;
  uint32_t get_tile_id(int x, int y) const;
  uint32_t get_tile_id_at(const Vector& pos) const;

  void change(int x, int y, uint32_t newtile);

  void change_at(const Vector& pos, uint32_t newtile);

  /** changes all tiles with the given ID */
  void change_all(uint32_t oldtile, uint32_t newtile);

  /** Puts the correct autotile block at the given position */
  void autotile(int x, int y, uint32_t tile);
  
  enum class AutotileCornerOperation {
    ADD_TOP_LEFT,
    ADD_TOP_RIGHT,
    ADD_BOTTOM_LEFT,
    ADD_BOTTOM_RIGHT,
    REMOVE_TOP_LEFT,
    REMOVE_TOP_RIGHT,
    REMOVE_BOTTOM_LEFT,
    REMOVE_BOTTOM_RIGHT,
  };
  
  /** Puts the correct autotile blocks at the tiles around the given corner */
  void autotile_corner(int x, int y, uint32_t tile, AutotileCornerOperation op);
  
  /** Erases in autotile mode */
  void autotile_erase(const Vector& pos, const Vector& corner_pos);

  /** Returns the Autotileset associated with the given tile */
  AutotileSet* get_autotileset(uint32_t tile) const;

  void set_flip(Flip flip) { m_flip = flip; }
  Flip get_flip() const { return m_flip; }

  /** Start fading the tilemap to opacity given by @c alpha.
      Destination opacity will be reached after @c seconds seconds.
      Also influences solidity. */
  void fade(float alpha, float seconds = 0);

  /** Start fading the tilemap to tint given by RGBA.
      Destination opacity will be reached after @c seconds seconds. Doesn't influence solidity. */
  void tint_fade(const Color& new_tint, float seconds = 0);

  /** Instantly switch tilemap's opacity to @c alpha. Also influences solidity. */
  void set_alpha(float alpha);

  /** Return tilemap's opacity. Note that while the tilemap is fading
      in or out, this will return the current alpha value, not the
      target alpha. */
  float get_alpha() const;

  void set_tileset(const TileSet* new_tileset);

  const std::vector<uint32_t>& get_tiles() const { return m_tiles; }
  
private:
  void update_effective_solid();
  void float_channel(float target, float &current, float remaining_time, float dt_sec);

  bool is_corner(uint32_t tile);

public:
  bool m_editor_active;

private:
  const TileSet* m_tileset;

  typedef std::vector<uint32_t> Tiles;
  Tiles m_tiles;

  /* read solid: In *general*, is this a solid layer? effective solid:
     is the layer *currently* solid? A generally solid layer may be
     not solid when its alpha is low. See `is_solid' above. */
  bool m_real_solid;
  bool m_effective_solid;

  float m_speed_x;
  float m_speed_y;
  int m_width;
  int m_height;
  int m_z_pos;
  Vector m_offset;
  Vector m_movement; /**< The movement that happened last frame */

  Flip m_flip;
  float m_alpha; /**< requested tilemap opacity */
  float m_current_alpha; /**< current tilemap opacity */
  float m_remaining_fade_time; /**< seconds until requested tilemap opacity is reached */

  /** The tint can have its own alpha channel, but this alpha channel doesn't affect
      the solidity of the tilemap. This alpha channel makes the tilemap only less or
      more translucent.*/
  Color m_tint; /**< requested tilemap tint */
  Color m_current_tint; /**< current tilemap tint */
  float m_remaining_tint_fade_time; /**< seconds until requested tilemap tint is reached */

  /** Is the tilemap currently moving (following the path) */
  bool m_running;

  /** Set to LIGHTMAP to draw to lightmap */
  DrawingTarget m_draw_target;

  int m_new_size_x;
  int m_new_size_y;
  int m_new_offset_x;
  int m_new_offset_y;
  bool m_add_path;

private:
  TileMap(const TileMap&) = delete;
  TileMap& operator=(const TileMap&) = delete;
};

#endif

/* EOF */
