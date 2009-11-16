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

#include "lisp/lisp.hpp"
#include "object/decal.hpp"
#include "supertux/object_factory.hpp"

Decal::Decal(const Reader& reader) :
  pos(),
  imagefile(),
  layer(LAYER_OBJECTS),
  image()
{
  float px = 0;
  float py = 0;
  reader.get("x", px);
  reader.get("y", py);
  pos = Vector(px, py);

  if(!reader.get("image", imagefile)) throw std::runtime_error("Must specify image for decal");
  image.reset(new Surface(imagefile));

  reader.get("layer", layer);
}

Decal::~Decal()
{
}

void
Decal::update(float)
{
}

void
Decal::draw(DrawingContext& context)
{
  if(!image.get()) return;
  context.draw_surface(image.get(), pos, layer);
}

IMPLEMENT_FACTORY(Decal, "decal");

/* EOF */
