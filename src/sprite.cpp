//  $Id$
//
//  SuperTux
//  Copyright (C) 2004 Ingo Ruhnke <grumbel@gmx.de>
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

#include <iostream>
#include <math.h>
#include "globals.h"
#include "sprite.h"
#include "setup.h"

Sprite::Sprite(lisp_object_t* cur)
{
  init_defaults();

  LispReader reader(cur);

  if(!reader.read_string("name",   &name))
    st_abort("Sprite wihtout name", "");
  reader.read_int("x-hotspot", &x_hotspot);
  reader.read_int("y-hotspot", &y_hotspot);
  reader.read_float("fps",     &fps);

  std::vector<std::string> images;
  if(!reader.read_string_vector("images", &images))
    st_abort("Sprite contains no images: ", name.c_str());

  for(std::vector<std::string>::size_type i = 0; i < images.size(); ++i)
    {
      surfaces.push_back(
          new Surface(datadir + "/images/" + images[i], USE_ALPHA));
    }        

  frame_delay = 1000.0f/fps;
}

Sprite::~Sprite()
{
  for(std::vector<Surface*>::iterator i = surfaces.begin(); i != surfaces.end();
      ++i)
    delete *i;
}

void
Sprite::init_defaults()
{
  x_hotspot = 0;
  y_hotspot = 0;
  fps = 10;
  time = 0;
  frame_delay = 1000.0f/fps;
}

void
Sprite::update(float /*delta*/)
{
  //time += 10*delta;
  //std::cout << "Delta: " << delta << std::endl;
}

void
Sprite::draw(float x, float y)
{
  time = SDL_GetTicks();
  unsigned int frame = get_current_frame();

  if (frame < surfaces.size())
    surfaces[frame]->draw(x - x_hotspot, y - y_hotspot);
}

void
Sprite::draw_part(float sx, float sy, float x, float y, float w, float h)
{
  time = SDL_GetTicks();
  unsigned int frame = get_current_frame();

  if (frame < surfaces.size())
    surfaces[frame]->draw_part(sx, sy, x - x_hotspot, y - y_hotspot, w, h);
}

void
Sprite::reset()
{
  time = 0;
}

int
Sprite::get_current_frame() const
{
  unsigned int frame = static_cast<int>(fmodf(time, surfaces.size()*frame_delay)/frame_delay);
  return frame % surfaces.size();
}

int
Sprite::get_width() const
{
  return surfaces[get_current_frame()]->w;
}

int
Sprite::get_height() const
{
  return surfaces[get_current_frame()]->h;
}

/* EOF */
