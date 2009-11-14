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

#ifndef SUPERTUX_GRADIENT_H
#define SUPERTUX_GRADIENT_H

#include <memory>
#include "video/surface.hpp"
#include "video/drawing_context.hpp"
#include "game_object.hpp"
#include "serializable.hpp"

class DisplayManager;

namespace lisp {
class Lisp;
}

class Gradient : public GameObject, public Serializable
{
public:
  Gradient();
  Gradient(const lisp::Lisp& reader);
  virtual ~Gradient();

  virtual void write(lisp::Writer& writer);

  void set_gradient(Color top, Color bottom);

  Color get_gradient_top() const
  { return gradient_top; }

  Color get_gradient_bottom() const
  { return gradient_bottom; }

  virtual void update(float elapsed_time);

  virtual void draw(DrawingContext& context);

private:
  int layer;
  Color gradient_top, gradient_bottom;
};

#endif /*SUPERTUX_BACKGROUND_H*/
