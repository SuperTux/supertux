//  SuperTux - Decal
//  Copyright (C) 2008 Christoph Sommer <christoph.sommer@2008.expires.deltadevelopment.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef SUPERTUX_DECAL_H
#define SUPERTUX_DECAL_H

#include <memory>
#include "video/surface.hpp"
#include "video/drawing_context.hpp"
#include "game_object.hpp"
#include "serializable.hpp"

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

