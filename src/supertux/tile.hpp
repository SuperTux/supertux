//  SuperTux
//  Copyright (C) 2004 Tobias Glaesser <tobi.web@gmx.de>
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//  Copyright (C) 2010 Florian Forster <supertux at octo.it>
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

#ifndef HEADER_SUPERTUX_SUPERTUX_TILE_HPP
#define HEADER_SUPERTUX_SUPERTUX_TILE_HPP

#include <vector>
#include <stdint.h>

#include "math/rectf.hpp"
#include "video/color.hpp"
#include "video/surface_ptr.hpp"

class Canvas;
class DrawingContext;

class Tile
{
public:
  static bool draw_editor_images;
  /// bitset for tile attributes
  enum {
    /** solid tile that is indestructible by Tux */
    SOLID     = 0x0001,
    /** uni-directional solid tile */
    UNISOLID  = 0x0002,
    /** a brick that can be destroyed by jumping under it */
    BRICK     = 0x0004, //Marked for removal, DO NOT USE!
    /** the level should be finished when touching a goaltile.
     * if data is 0 then the endsequence should be triggered, if data is 1
     * then we can finish the level instantly.
     */
    GOAL      = 0x0008, //Marked for removal, DO NOT USE!
    /** slope tile */
    SLOPE     = 0x0010,
    /** Bonusbox, content is stored in \a data */
    FULLBOX   = 0x0020, //Marked for removal, DO NOT USE!
    /** Tile is a coin */
    COIN      = 0x0040, //Marked for removal, DO NOT USE!

    /* interesting flags (the following are passed to gameobjects) */
    FIRST_INTERESTING_FLAG = 0x0100,

    /** an ice brick that makes tux sliding more than usual */
    ICE       = 0x0100,
    /** a water tile in which tux starts to swim */
    WATER     = 0x0200,
    /** a tile that hurts the player if he touches it */
    HURTS     = 0x0400,
    /** for lava: WATER, HURTS, FIRE */
    FIRE      = 0x0800
  };

  /// worldmap flags
  enum {
    WORLDMAP_NORTH = 0x0001,
    WORLDMAP_SOUTH = 0x0002,
    WORLDMAP_EAST  = 0x0004,
    WORLDMAP_WEST  = 0x0008,
    WORLDMAP_DIR_MASK = 0x000f,

    WORLDMAP_STOP  = 0x0010,

    // convenience values ("C" stands for crossroads)
    WORLDMAP_CNSE  = WORLDMAP_NORTH | WORLDMAP_SOUTH | WORLDMAP_EAST,
    WORLDMAP_CNSW  = WORLDMAP_NORTH | WORLDMAP_SOUTH | WORLDMAP_WEST,
    WORLDMAP_CNEW  = WORLDMAP_NORTH | WORLDMAP_EAST  | WORLDMAP_WEST,
    WORLDMAP_CSEW  = WORLDMAP_SOUTH | WORLDMAP_EAST  | WORLDMAP_WEST,
    WORLDMAP_CNSEW = WORLDMAP_NORTH | WORLDMAP_SOUTH | WORLDMAP_EAST | WORLDMAP_WEST
  };

  struct ImageSpec {
    ImageSpec(const std::string& newfile, const Rectf& newrect)
      : file(newfile), rect(newrect)
    { }

    std::string file;
    Rectf rect;
  };

  enum
  {
    UNI_DIR_NORTH = 0,
    UNI_DIR_SOUTH = 1,
    UNI_DIR_WEST  = 2,
    UNI_DIR_EAST  = 3,
    UNI_DIR_MASK  = 3
  };

private:
  std::vector<ImageSpec> imagespecs;
  std::vector<SurfacePtr>  images;
  std::vector<ImageSpec> editor_imagespecs;
  std::vector<SurfacePtr>  editor_images;

  /// tile attributes
  uint32_t attributes;

  /** General purpose data attached to a tile (content of a box, type of coin)*/
  int data;

  float fps;

  std::string object_name;
  std::string object_data;

public:
  Tile();
  Tile(const std::vector<ImageSpec>& images, const std::vector<ImageSpec>& editor_images,
       uint32_t attributes, uint32_t data, float fps, const std::string& obj_name = "",
       const std::string& obj_data = "");

  /** load Surfaces, if not already loaded */
  void load_images();

  SurfacePtr get_current_image() const;

  /** Draw a tile on the screen */
  void draw(Canvas& canvas, const Vector& pos, int z_pos, Color color = Color(1, 1, 1)) const;

  uint32_t getAttributes() const
  { return attributes; }

  int getData() const
  { return data; }

  /** Checks the SLOPE attribute. Returns "true" if set, "false" otherwise. */
  bool is_slope() const
  {
    return attributes & SLOPE;
  }

  /** Determine the solidity of a tile. This version behaves correctly for
   * unisolid tiles by taking position and movement of the object in question
   * into account. Because creating the arguments for this function can be
   * expensive, you should handle trivial cases using the "is_solid()" and
   * "is_unisolid()" methods first. */
  bool is_solid (const Rectf& tile_bbox, const Rectf& position, const Vector& movement) const;

  /** This version only checks the SOLID flag to determine the solidity of a
   * tile. This means it will always return "true" for unisolid tiles. To
   * determine the *current* solidity of unisolid tiles, use the "is_solid"
   * method that takes position and movement into account (see above). */
  bool is_solid() const
  {
    return attributes & SOLID;
  }

  /** Determines whether the tile's attributes are important to calculate the
   * collisions. The tile may be unisolid and therefore the collision with that
   * tile don't matter.*/
  bool is_collisionful(const Rectf& tile_bbox, const Rectf& position, const Vector& movement) const;

  /** Checks the UNISOLID attribute. Returns "true" if set, "false" otherwise. */
  bool is_unisolid() const
  {
    return attributes & UNISOLID;
  }

  std::string get_object_name() const {
    return object_name;
  }

  std::string get_object_data() const {
    return object_data;
  }

  void print_debug(int id) const;

private:
  //Correct small oddities in attributes that naive people
  //might miss (and rebuke them for it)
  void correct_attributes();

  /** Returns zero if a unisolid tile is non-solid due to the movement
   * direction, non-zero if the tile is solid due to direction. */
  bool check_movement_unisolid (const Vector& movement) const;

  /** Returns zero if a unisolid tile is non-solid due to the position of the
   * tile and the object, non-zero if the tile is solid. */
  bool check_position_unisolid (const Rectf& obj_bbox,
                                const Rectf& tile_bbox) const;

private:
  Tile(const Tile&);
  Tile& operator=(const Tile&);
};

#endif

/* EOF */
