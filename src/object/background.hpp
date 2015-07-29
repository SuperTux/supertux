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

#ifndef HEADER_SUPERTUX_OBJECT_BACKGROUND_HPP
#define HEADER_SUPERTUX_OBJECT_BACKGROUND_HPP

#include "supertux/game_object.hpp"
#include "supertux/script_interface.hpp"
#include "util/reader_fwd.hpp"
#include "video/drawing_context.hpp"

class Background : public GameObject,
                   public ScriptInterface
{
public:
  Background();
  Background(const ReaderMapping& reader);
  virtual ~Background();
  virtual void save(lisp::Writer& writer);

  void set_image(const std::string& name);
  void set_image(const std::string& name, float bkgd_speed);
  void set_images(const std::string& name_top_, const std::string& name_middle_, const std::string& name_bottom_);

  std::string get_image() const
  { return imagefile; }
  float get_speed() const
  { return speed; }

  virtual void update(float elapsed_time);

  virtual void draw(DrawingContext& context);
  void draw_image(DrawingContext& context, const Vector& pos);

  virtual void expose(HSQUIRRELVM vm, SQInteger table_idx);
  virtual void unexpose(HSQUIRRELVM vm, SQInteger table_idx);

  virtual std::string get_class() {
    return "background";
  }

private:
  enum Alignment {
    NO_ALIGNMENT,
    LEFT_ALIGNMENT,
    RIGHT_ALIGNMENT,
    TOP_ALIGNMENT,
    BOTTOM_ALIGNMENT
  };

  /** Backgrounds with NO_ALIGNMENT are repeated over the whole
      screen, backgrounds with left, right, top, bottom alignment are
      only repeated in one direction and attached to the level edge */
  Alignment alignment;

  int layer;
  std::string imagefile_top;
  std::string imagefile;
  std::string imagefile_bottom;
  Vector pos; /**< coordinates of upper-left corner of image */
  float speed; /**< scroll-speed in horizontal direction */
  float speed_y; /**< scroll-speed in vertical direction */
  Vector scroll_speed;
  Vector scroll_offset;
  SurfacePtr image_top; /**< image to draw above pos */
  SurfacePtr image; /**< image to draw, anchored at pos */
  SurfacePtr image_bottom; /**< image to draw below pos+screenheight */

  bool has_pos_x, has_pos_y;
};

#endif /*SUPERTUX_BACKGROUND_H*/

/* EOF */
