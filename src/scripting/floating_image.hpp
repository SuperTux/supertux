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

#ifndef HEADER_SUPERTUX_SCRIPTING_FLOATING_IMAGE_HPP
#define HEADER_SUPERTUX_SCRIPTING_FLOATING_IMAGE_HPP

#ifndef SCRIPTING_API
#include <memory>
#include "scripting/sqrat_object.hpp"

class FloatingImage;
#endif

namespace scripting {

#ifdef SCRIPTING_API
class FloatingImage
#else
class FloatingImage: SQRatObject<FloatingImage>
#endif
{
public:
#ifndef SCRIPTING_API
  FloatingImage(const std::string& spritefile);
  ~FloatingImage();
  static void register_exposed_methods(HSQUIRRELVM v, SQRatClassType squirrelClass)
  {
    squirrelClass.Func("set_layer", &FloatingImage::set_layer);
    squirrelClass.Func("get_layer", &FloatingImage::get_layer);
    squirrelClass.Func("set_pos", &FloatingImage::set_pos);
    squirrelClass.Func("get_pos_x", &FloatingImage::get_pos_x);
    squirrelClass.Func("get_pos_y", &FloatingImage::get_pos_y);
    squirrelClass.Func("set_anchor_point", &FloatingImage::set_anchor_point);
    squirrelClass.Func("get_anchor_point", &FloatingImage::get_anchor_point);
    squirrelClass.Func("set_visible", &FloatingImage::set_visible);
    squirrelClass.Func("get_visible", &FloatingImage::get_visible);
    squirrelClass.Func("set_action", &FloatingImage::set_action);
    squirrelClass.Func("get_action", &FloatingImage::get_action);
    squirrelClass.Func("fade_in", &FloatingImage::fade_in);
    squirrelClass.Func("fade_out", &FloatingImage::fade_out);
  }
#endif

  void set_layer(int layer);
  int get_layer() const;
  void set_pos(float x, float y);
  float get_pos_x() const;
  float get_pos_y() const;
  void set_anchor_point(int anchor);
  int get_anchor_point() const;
  void set_visible(bool visible);
  bool get_visible() const;
  void set_action(const std::string& action);
  std::string get_action() const;
  void fade_in(float fadetime);
  void fade_out(float fadetime);

#ifndef SCRIPTING_API
private:
  std::shared_ptr<::FloatingImage> floating_image;
#endif
};

}

#endif

/* EOF */
