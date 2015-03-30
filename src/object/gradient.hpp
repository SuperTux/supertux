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

#ifndef HEADER_SUPERTUX_OBJECT_GRADIENT_HPP
#define HEADER_SUPERTUX_OBJECT_GRADIENT_HPP

#include "supertux/game_object.hpp"
#include "util/reader_fwd.hpp"
#include "video/drawing_context.hpp"

class DisplayManager;

class Gradient : public GameObject
{
public:
  Gradient();
  Gradient(const Reader& reader);
  virtual ~Gradient();

  void set_gradient(Color top, Color bottom);

  Color get_gradient_top() const
  { return gradient_top; }

  Color get_gradient_bottom() const
  { return gradient_bottom; }

  GradientDirection get_gradient_direction() const
  { return gradient_direction; }

  virtual void update(float elapsed_time);

  virtual void draw(DrawingContext& context);

private:
  int layer;
  Color gradient_top;
  Color gradient_bottom;
  GradientDirection gradient_direction;
};

#endif /*SUPERTUX_BACKGROUND_H*/

/* EOF */
