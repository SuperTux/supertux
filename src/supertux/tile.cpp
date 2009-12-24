//  SuperTux
//  Copyright (C) 2004 Tobias Glaesser <tobi.web@gmx.de>
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

#include "supertux/tile.hpp"

#include "supertux/tile_set.hpp"
#include "video/drawing_context.hpp"

Tile::Tile(const TileSet& new_tileset) :
  tileset(new_tileset), 
  imagespecs(),
  images(),
  attributes(0), 
  data(0), 
  fps(1)
{
}

Tile::Tile(const TileSet& new_tileset, const std::vector<ImageSpec>& imagespecs_, 
           uint32_t attributes, uint32_t data, float fps) :
  tileset(new_tileset),
  imagespecs(imagespecs_),
  images(),
  attributes(attributes), 
  data(data), 
  fps(fps)
{
  correct_attributes();
}

Tile::~Tile()
{
}

void
Tile::load_images()
{
  if(images.size() == 0 && imagespecs.size() != 0)
  {
    assert(images.size() == 0);
    for(std::vector<ImageSpec>::iterator i = imagespecs.begin(); i != imagespecs.end(); ++i) 
    {
      const ImageSpec& spec = *i;

      SurfacePtr surface;
      if(spec.rect.get_width() <= 0) 
      {
        surface = Surface::create(spec.file);
      }
      else 
      {
        surface = Surface::create(spec.file,
                                  Rect((int) spec.rect.p1.x,
                                       (int) spec.rect.p1.y,
                                       Size((int) spec.rect.get_width(),
                                            (int) spec.rect.get_height())));
      }
      images.push_back(surface);
    }
  }
}

void
Tile::draw(DrawingContext& context, const Vector& pos, int z_pos) const
{
  if(images.size() > 1) {
    size_t frame = size_t(game_time * fps) % images.size();
    context.draw_surface(images[frame], pos, z_pos);
  } else if (images.size() == 1) {
    context.draw_surface(images[0], pos, z_pos);
  }
}

void
Tile::correct_attributes()
{
  //Fix little oddities in attributes (not many, currently...)
  if(!(attributes & SOLID) && (attributes & SLOPE || attributes & UNISOLID)) {
    attributes |= SOLID;
    //But still be vocal about it
    log_warning << "Tile with image " << imagespecs[0].file << " needs solid attribute." << std::endl;
  }
}

void
Tile::print_debug(int id) const
{
  log_debug << " Tile: id " << id << ", data " << getData() << ", attributes " << getAttributes() << ":" << std::endl;
  for(std::vector<Tile::ImageSpec>::const_iterator im = imagespecs.begin(); im != imagespecs.end(); ++im) 
  {
    log_debug << "  Imagespec: file " << im->file << "; rect " << im->rect << std::endl;
  }
}

/* EOF */
