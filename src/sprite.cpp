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
#include "globals.h"
#include "sprite.h"

Sprite::Sprite(lisp_object_t* cur)
{
  init_defaults();

  LispReader reader(cur);

  reader.read_int("x-hotspot", &x_hotspot);
  reader.read_int("y-hotspot", &y_hotspot);
  reader.read_float("fps", &fps);
  std::vector<std::string> images;
  reader.read_string_vector("images", &images);
  surfaces.resize(images.size());

  for(std::vector<std::string>::size_type i = 0; i < images.size(); ++i)
    {
      texture_load(&surfaces[i], datadir + "/images/" + images[i], USE_ALPHA);
    }        
}

void
Sprite::init_defaults()
{
  x_hotspot = 0;
  y_hotspot = 0;
  fps = 15;
  time = 0;
  frame_delay = 1000.0f/fps;
}

void
Sprite::update(float delta)
{
  time += 10*delta;
  //std::cout << "Delta: " << delta << std::endl;
}

void
Sprite::draw(int x, int y)
{
  unsigned int frame = static_cast<int>(fmodf(time, surfaces.size()*frame_delay)/frame_delay);
  
  /*
  std::cout << "Frame: "
            << frame << " "
            << time << " "
            << surfaces.size() << " "
            << frame_delay << " "
            << static_cast<int>(fmodf(time, surfaces.size()*frame_delay)/frame_delay) << std::endl;*/
  if (frame < surfaces.size())
    texture_draw(&surfaces[frame], x - x_hotspot, y - y_hotspot);
}

/* EOF */
