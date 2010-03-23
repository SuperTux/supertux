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

#include "supertux/tile.hpp"

#include "supertux/constants.hpp"
#include "supertux/tile_set.hpp"
#include "math/aatriangle.hpp"
#include "video/drawing_context.hpp"

bool Tile::draw_editor_images = false;

Tile::Tile() :
  imagespecs(),
  images(),
  editor_imagespecs(),
  editor_images(),
  attributes(0), 
  data(0), 
  fps(1)
{
}

Tile::Tile(const std::vector<ImageSpec>& imagespecs_, const std::vector<ImageSpec>& editor_imagespecs_, 
           uint32_t attributes, uint32_t data, float fps) :
  imagespecs(imagespecs_),
  images(),
  editor_imagespecs(editor_imagespecs_),
  editor_images(),
  attributes(attributes), 
  data(data), 
  fps(fps)
{
  correct_attributes();
}

Tile::~Tile()
{
}

void
Tile::load_images()
{
  if(images.size() == 0 && imagespecs.size() != 0)
  {
    assert(images.size() == 0);
    for(std::vector<ImageSpec>::iterator i = imagespecs.begin(); i != imagespecs.end(); ++i) 
    {
      const ImageSpec& spec = *i;

      SurfacePtr surface;
      if(spec.rect.get_width() <= 0) 
      {
        surface = Surface::create(spec.file);
      }
      else 
      {
        surface = Surface::create(spec.file,
                                  Rect((int) spec.rect.p1.x,
                                       (int) spec.rect.p1.y,
                                       Size((int) spec.rect.get_width(),
                                            (int) spec.rect.get_height())));
      }
      images.push_back(surface);
    }
  }

  if(editor_images.size() == 0 && editor_imagespecs.size() != 0)
  {
    assert(editor_images.size() == 0);
    for(std::vector<ImageSpec>::iterator i = editor_imagespecs.begin(); i != editor_imagespecs.end(); ++i) 
    {
      const ImageSpec& spec = *i;

      SurfacePtr surface;
      if(spec.rect.get_width() <= 0) 
      {
        surface = Surface::create(spec.file);
      }
      else 
      {
        surface = Surface::create(spec.file,
                                  Rect((int) spec.rect.p1.x,
                                       (int) spec.rect.p1.y,
                                       Size((int) spec.rect.get_width(),
                                            (int) spec.rect.get_height())));
      }
      editor_images.push_back(surface);
    }
  }
}

void
Tile::draw(DrawingContext& context, const Vector& pos, int z_pos) const
{
  if(draw_editor_images) {
    if(editor_images.size() > 1) {
      size_t frame = size_t(game_time * fps) % editor_images.size();
      context.draw_surface(editor_images[frame], pos, z_pos);
      return;
    } else if (editor_images.size() == 1) {
      context.draw_surface(editor_images[0], pos, z_pos);
      return;
    }
  }

  if(images.size() > 1) {
    size_t frame = size_t(game_time * fps) % images.size();
    context.draw_surface(images[frame], pos, z_pos);
  } else if (images.size() == 1) {
    context.draw_surface(images[0], pos, z_pos);
  }
}

void
Tile::correct_attributes()
{
  //Fix little oddities in attributes (not many, currently...)
  if(!(attributes & SOLID) && (attributes & SLOPE || attributes & UNISOLID)) {
    attributes |= SOLID;
    //But still be vocal about it
    log_warning << "Tile with image " << imagespecs[0].file << " needs solid attribute." << std::endl;
  }
}

void
Tile::print_debug(int id) const
{
  log_debug << " Tile: id " << id << ", data " << getData() << ", attributes " << getAttributes() << ":" << std::endl;
  for(std::vector<Tile::ImageSpec>::const_iterator im = editor_imagespecs.begin(); im != editor_imagespecs.end(); ++im) 
    log_debug << "  Editor Imagespec: file " << im->file << "; rect " << im->rect << std::endl;
  for(std::vector<Tile::ImageSpec>::const_iterator im = imagespecs.begin(); im != imagespecs.end(); ++im) 
    log_debug << "  Imagespec:        file " << im->file << "; rect " << im->rect << std::endl;
}

/* Check if the tile is solid given the current movement. This works
 * for south-slopes (which are solid when moving "down") and
 * north-slopes (which are solid when moving "up". "up" and "down" is
 * in quotation marks because because the slope's gradient is taken.
 * Also, this uses the movement relative to the tilemaps own movement
 * (if any).  --octo */
bool Tile::check_movement_unisolid (const Vector movement) const
{
  int slope_info;
  double mv_x;
  double mv_y;
  double mv_tan;
  double slope_tan;

  /* If the tile is not a slope, this is very easy. */
  if (!this->is_slope())
  {
    int dir = this->getData() & Tile::UNI_DIR_MASK;

    return ((dir == Tile::UNI_DIR_NORTH) && (movement.y >= 0))  /* moving down */
        || ((dir == Tile::UNI_DIR_SOUTH) && (movement.y <= 0))  /* moving up */
        || ((dir == Tile::UNI_DIR_WEST ) && (movement.x >= 0))  /* moving right */
        || ((dir == Tile::UNI_DIR_EAST ) && (movement.x <= 0)); /* moving left */
  }

  /* Initialize mv_x and mv_y. Depending on the slope the axis are inverted so
   * that we can always use the "SOUTHEAST" case of the slope. The southeast
   * case is the following:
   *     .
   *    /!
   *   / !
   *  +--+
   */
  mv_x = (double) movement.x; //note switch to double for no good reason
  mv_y = (double) movement.y;

  slope_info = this->getData();
  switch (slope_info & AATriangle::DIRECTION_MASK)
  {
    case AATriangle::SOUTHEAST: /*    . */
      /* do nothing */          /*   /! */
      break;                    /*  / ! */
                                /* +--+ */
    case AATriangle::SOUTHWEST: /* .    */
      mv_x *= (-1.0);           /* !\   */
      break;                    /* ! \  */
                                /* +--+ */
    case AATriangle::NORTHEAST: /* +--+ */
      mv_y *= (-1.0);           /*  \ ! */
      break;                    /*   \! */
                                /*    ' */
    case AATriangle::NORTHWEST: /* +--+ */
      mv_x *= (-1.0);           /* ! /  */
      mv_y *= (-1.0);           /* !/   */
      break;                    /* '    */
  } /* switch (slope_info & DIRECTION_MASK) */

  /* Handle the easy cases first */
  /* If we're moving to the right and down, then the slope is solid. */
  if ((mv_x >= 0.0) && (mv_y >= 0.0)) /* 4th quadrant */
    return true;
  /* If we're moving to the left and up, then the slope is not solid. */
  else if ((mv_x <= 0.0) && (mv_y <= 0.0)) /* 2nd quadrant */
    return false;

  /* The pure up-down and left-right movements have already been handled. */
  assert (mv_x != 0.0);
  assert (mv_y != 0.0);

  /* calculate tangent of movement */
  mv_tan = (-1.0) * mv_y / mv_x;

  /* determine tangent of the slope */
  slope_tan = 1.0;
  if (((slope_info & AATriangle::DEFORM_MASK) == AATriangle::DEFORM_BOTTOM)
      || ((slope_info & AATriangle::DEFORM_MASK) == AATriangle::DEFORM_TOP))
    slope_tan = 0.5; /* ~= 26.6 deg */
  else if (((slope_info & AATriangle::DEFORM_MASK) == AATriangle::DEFORM_LEFT)
      || ((slope_info & AATriangle::DEFORM_MASK) == AATriangle::DEFORM_RIGHT))
    slope_tan = 2.0; /* ~= 63.4 deg */

  /* up and right */
  if (mv_x > 0.0) /* 1st quadrant */
  {
    assert (mv_y < 0.0);
    return (mv_tan <= slope_tan);
  }
  /* down and left */
  else if (mv_x < 0.0) /* 3rd quadrant */
  {
    assert (mv_y > 0.0);
    return (mv_tan >= slope_tan);
  }

  assert (1 != 1);
  return false;
} /* int check_movement_unisolid */

bool is_above_line (float l_x, float l_y, float m,
                    float p_x, float p_y)
{
  float interp_y = (l_y + (m * (p_x - l_x)));
  return (interp_y >= p_y);
}

bool is_below_line (float l_x, float l_y, float m,
                    float p_x, float p_y)
{
  return !is_above_line (l_x, l_y, m, p_x, p_y);
}

/* Check whether the object is already *in* the tile. If so, the tile
 * is non-solid. Otherwise, if the object is "above" (south slopes)
 * or "below" (north slopes), the tile will be solid. */
bool Tile::check_position_unisolid (const Rectf& obj_bbox,
                                    const Rectf& tile_bbox) const
{
  int slope_info;
  float tile_x;
  float tile_y;
  float gradient;
  float delta_x;
  float delta_y;
  float obj_x;
  float obj_y;

  /* If this is not a slope, this is - again - easy */
  if (!this->is_slope())
  {
    int dir = this->getData() & Tile::UNI_DIR_MASK;

    return ((dir == Tile::UNI_DIR_NORTH) && ((obj_bbox.get_bottom() - SHIFT_DELTA) <= tile_bbox.get_top()   ))
        || ((dir == Tile::UNI_DIR_SOUTH) && ((obj_bbox.get_top()    + SHIFT_DELTA) >= tile_bbox.get_bottom()))
        || ((dir == Tile::UNI_DIR_WEST ) && ((obj_bbox.get_right()  - SHIFT_DELTA) <= tile_bbox.get_left()  ))
        || ((dir == Tile::UNI_DIR_EAST ) && ((obj_bbox.get_left()   + SHIFT_DELTA) >= tile_bbox.get_right() ));
  }

  /* There are 20 different cases. For each case, calculate a line that
   * describes the slope's surface. The line is defined by x, y, and m, the
   * gradient. */
  slope_info = this->getData();
  switch (slope_info
      & (AATriangle::DIRECTION_MASK | AATriangle::DEFORM_MASK))
  {
    case AATriangle::SOUTHWEST:
    case AATriangle::SOUTHWEST | AATriangle::DEFORM_TOP:
    case AATriangle::SOUTHWEST | AATriangle::DEFORM_LEFT:
    case AATriangle::NORTHEAST:
    case AATriangle::NORTHEAST | AATriangle::DEFORM_TOP:
    case AATriangle::NORTHEAST | AATriangle::DEFORM_LEFT:
      tile_x = tile_bbox.get_left();
      tile_y = tile_bbox.get_top();
      gradient = 1.0;
      break;

    case AATriangle::SOUTHEAST:
    case AATriangle::SOUTHEAST | AATriangle::DEFORM_TOP:
    case AATriangle::SOUTHEAST | AATriangle::DEFORM_RIGHT:
    case AATriangle::NORTHWEST:
    case AATriangle::NORTHWEST | AATriangle::DEFORM_TOP:
    case AATriangle::NORTHWEST | AATriangle::DEFORM_RIGHT:
      tile_x = tile_bbox.get_right();
      tile_y = tile_bbox.get_top();
      gradient = -1.0;
      break;

    case AATriangle::SOUTHEAST | AATriangle::DEFORM_BOTTOM:
    case AATriangle::SOUTHEAST | AATriangle::DEFORM_LEFT:
    case AATriangle::NORTHWEST | AATriangle::DEFORM_BOTTOM:
    case AATriangle::NORTHWEST | AATriangle::DEFORM_LEFT:
      tile_x = tile_bbox.get_left();
      tile_y = tile_bbox.get_bottom();
      gradient = -1.0;
      break;

    case AATriangle::SOUTHWEST | AATriangle::DEFORM_BOTTOM:
    case AATriangle::SOUTHWEST | AATriangle::DEFORM_RIGHT:
    case AATriangle::NORTHEAST | AATriangle::DEFORM_BOTTOM:
    case AATriangle::NORTHEAST | AATriangle::DEFORM_RIGHT:
      tile_x = tile_bbox.get_right();
      tile_y = tile_bbox.get_bottom();
      gradient = 1.0;
      break;

    default:
      assert (23 == 42);
      return 0;
  }

  /* delta_x, delta_y: Gradient aware version of SHIFT_DELTA. Here, we set the
   * sign of the values only. Also, we determine here which corner of the
   * object's bounding box is the interesting one for us. */
  delta_x = 1.0 * SHIFT_DELTA;
  delta_y = 1.0 * SHIFT_DELTA;
  switch (slope_info & AATriangle::DIRECTION_MASK)
  {
    case AATriangle::SOUTHWEST:
      delta_x *= 1.0;
      delta_y *= -1.0;
      obj_x = obj_bbox.get_left();
      obj_y = obj_bbox.get_bottom();
      break;

    case AATriangle::SOUTHEAST:
      delta_x *= -1.0;
      delta_y *= -1.0;
      obj_x = obj_bbox.get_right();
      obj_y = obj_bbox.get_bottom();
      break;

    case AATriangle::NORTHWEST:
      delta_x *= 1.0;
      delta_y *= 1.0;
      obj_x = obj_bbox.get_left();
      obj_y = obj_bbox.get_top();
      break;

    case AATriangle::NORTHEAST:
      delta_x *= -1.0;
      delta_y *= 1.0;
      obj_x = obj_bbox.get_right();
      obj_y = obj_bbox.get_top();
      break;
  }

  /* Adapt the delta_x, delta_y and the gradient for the 26.6 deg and 63.4 deg
   * cases. */
  switch (slope_info & AATriangle::DEFORM_MASK)
  {
    case 0:
      delta_x *= .70710678118654752440; /* 1/sqrt(2) */
      delta_y *= .70710678118654752440; /* 1/sqrt(2) */
      break;

    case AATriangle::DEFORM_BOTTOM:
    case AATriangle::DEFORM_TOP:
      delta_x *= .44721359549995793928; /* 1/sqrt(5) */
      delta_y *= .89442719099991587856; /* 2/sqrt(5) */
      gradient *= 0.5;
      break;

    case AATriangle::DEFORM_LEFT:
    case AATriangle::DEFORM_RIGHT:
      delta_x *= .89442719099991587856; /* 2/sqrt(5) */
      delta_y *= .44721359549995793928; /* 1/sqrt(5) */
      gradient *= 2.0;
      break;
  }

  /* With a south slope, check if all points are above the line. If one point
   * isn't, the slope is not solid. => You can pass through a south-slope from
   * below but not from above. */
  if (((slope_info & AATriangle::DIRECTION_MASK) == AATriangle::SOUTHWEST)
   || ((slope_info & AATriangle::DIRECTION_MASK) == AATriangle::SOUTHEAST))
  {
    return !is_below_line(tile_x, tile_y, gradient, obj_x + delta_x, obj_y + delta_y);
  }
  /* northwest or northeast. Same as above, but inverted. You can pass from top
   * to bottom but not vice versa. */
  else
  {
    return !is_above_line (tile_x, tile_y, gradient, obj_x + delta_x, obj_y + delta_y);
  }
} /* int check_position_unisolid */

bool Tile::is_solid (const Rectf& tile_bbox, const Rectf& position, const Vector& movement) const
{
  if (!(attributes & SOLID))
    return false;

  if (!(attributes & UNISOLID))
    return true;

  return check_movement_unisolid (movement) &&
         check_position_unisolid (position, tile_bbox);
} /* bool Tile::is_solid */

/* vim: set sw=2 sts=2 et : */
/* EOF */
