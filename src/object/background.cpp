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

#include "object/background.hpp"
#include "supertux/globals.hpp"
#include "supertux/object_factory.hpp"
#include "util/reader.hpp"

Background::Background() :
  layer(LAYER_BACKGROUND0),
  imagefile_top(),
  imagefile(),
  imagefile_bottom(),
  pos(),
  speed(),
  speed_y(),
  image_top(),
  image(),
  image_bottom()
{
}

Background::Background(const Reader& reader) :
  layer(LAYER_BACKGROUND0),
  imagefile_top(),
  imagefile(),
  imagefile_bottom(),
  pos(),
  speed(),
  speed_y(),
  image_top(),
  image(),
  image_bottom()
{
  // read position, defaults to (0,0)
  float px = 0;
  float py = 0;
  reader.get("x", px);
  reader.get("y", py);
  this->pos = Vector(px,py);

  speed = 1.0;
  speed_y = 1.0;

  reader.get("layer", layer);
  if(!reader.get("image", imagefile) || !reader.get("speed", speed))
    throw std::runtime_error("Must specify image and speed for background");

  set_image(imagefile, speed);
  if (!reader.get("speed-y", speed_y))
  {
    speed_y = speed;
  }

  if (reader.get("image-top", imagefile_top)) {
    image_top = Surface::create(imagefile_top);
  }
  if (reader.get("image-bottom", imagefile_bottom)) {
    image_bottom = Surface::create(imagefile_bottom);
  }
}

Background::~Background()
{
}

void
Background::update(float)
{
}

void
Background::set_image(const std::string& name, float speed)
{
  this->imagefile = name;
  this->speed = speed;

  image = Surface::create(name);
}

void
Background::draw(DrawingContext& context)
{
  if(image.get() == NULL)
    return;

  int w = (int) image->get_width();
  int h = (int) image->get_height();
  int sx = int(pos.x-context.get_translation().x * speed) % w - w;
  int sy = int(pos.y-context.get_translation().y * speed_y) % h - h;
  int center_image_py = int(pos.y-context.get_translation().y * speed_y);
  int bottom_image_py = int(pos.y-context.get_translation().y * speed_y) + h;
  context.push_transform();
  context.set_translation(Vector(0, 0));
  for(int x = sx; x < SCREEN_WIDTH; x += w) {
    for(int y = sy; y < SCREEN_HEIGHT; y += h) {
      if (image_top.get() != NULL && (y < center_image_py)) {
        context.draw_surface(image_top.get(), Vector(x, y), layer);
        continue;
      }
      if (image_bottom.get() != NULL && (y >= bottom_image_py)) {
        context.draw_surface(image_bottom.get(), Vector(x, y), layer);
        continue;
      }
      context.draw_surface(image.get(), Vector(x, y), layer);
    }
  }
  context.pop_transform();
}

/* EOF */
