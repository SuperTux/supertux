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

#ifndef __FLOATING_IMAGE_HPP__
#define __FLOATING_IMAGE_HPP__

#ifndef SCRIPTING_API
#define __suspend
#include <string>
#include "util/ref.hpp"

class FloatingImage;
typedef FloatingImage _FloatingImage;
#endif

namespace Scripting
{

class FloatingImage
{
public:
  FloatingImage(const std::string& spritefile);
  ~FloatingImage();

  void set_layer(int layer);
  int get_layer();
  void set_pos(float x, float y);
  float get_pos_x();
  float get_pos_y();
  void set_anchor_point(int anchor);
  int get_anchor_point();
  void set_visible(bool visible);
  bool get_visible();
  void set_action(const std::string& action);
  std::string get_action();
  void fade_in(float fadetime);
  void fade_out(float fadetime);

#ifndef SCRIPTING_API
private:
  Ref<_FloatingImage> floating_image;
#endif
};

}

#endif
