//  SuperTux - Decal
//  Copyright (C) 2008 Christoph Sommer <christoph.sommer@2008.expires.deltadevelopment.de>
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

#ifndef HEADER_SUPERTUX_OBJECT_DECAL_HPP
#define HEADER_SUPERTUX_OBJECT_DECAL_HPP

#include <memory>

#include "supertux/game_object.hpp"
#include "util/serializable.hpp"
#include "video/drawing_context.hpp"
#include "video/surface.hpp"

class DisplayManager;

namespace lisp {
class Lisp;
}

/**
 * Non-interactive, decorative image
 */
class Decal : public GameObject, public Serializable
{
public:
  Decal(const lisp::Lisp& reader);
  virtual ~Decal();

  virtual void write(lisp::Writer& writer);
  virtual void update(float elapsed_time);
  virtual void draw(DrawingContext& context);

private:
  Vector pos; /**< coordinates of upper-left corner */
  std::string imagefile; /**< name of image to draw */
  int layer; /**< layer to draw on */

  std::auto_ptr<Surface> image; /**< image to draw, anchored at pos */
};

#endif /*SUPERTUX_DECAL_H*/

