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

#include "object/gradient.hpp"
#include "object/camera.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"
#include "util/gettext.hpp"
#include "util/reader.hpp"

#include <stdexcept>

Gradient::Gradient() :
  layer(LAYER_BACKGROUND0),
  gradient_top(),
  gradient_bottom(),
  gradient_direction(),
  gradient_region(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT)
{
}

Gradient::Gradient(const Reader& reader) :
  layer(LAYER_BACKGROUND0),
  gradient_top(),
  gradient_bottom(),
  gradient_direction(),
  gradient_region(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT)
{
  layer = reader_get_layer (reader, /* default = */ LAYER_BACKGROUND0);
  std::vector<float> bkgd_top_color, bkgd_bottom_color;
  std::string direction;
  if(reader.get("direction", direction))
  {
    if(direction == "horizontal")
    {
      gradient_direction = HORIZONTAL;
    }
    else if(direction == "horizontal_sector")
    {
        gradient_direction = HORIZONTAL_SECTOR;
    }
    else if(direction == "vertical_sector")
    {
        gradient_direction = VERTICAL_SECTOR;
    }
    else
    {
        gradient_direction = VERTICAL;
    }
  }
  else
  {
    gradient_direction = VERTICAL;
  }
  if(gradient_direction == HORIZONTAL || gradient_direction == HORIZONTAL_SECTOR)
  {
    if(!reader.get("left_color", bkgd_top_color) ||
       !reader.get("right_color", bkgd_bottom_color))
    {
      log_warning << "Horizontal gradients should use left_color and right_color, respectively. Trying to parse top and bottom color instead" << std::endl;
    }
    else
    {
      gradient_top = Color(bkgd_top_color);
      gradient_bottom = Color(bkgd_bottom_color);
      return;
    }
  }

  if(!reader.get("top_color", bkgd_top_color) ||
     !reader.get("bottom_color", bkgd_bottom_color))
    throw std::runtime_error("Must specify top_color and bottom_color in gradient");

  gradient_top = Color(bkgd_top_color);
  gradient_bottom = Color(bkgd_bottom_color);
}

void
Gradient::save(lisp::Writer& writer) {
  GameObject::save(writer);
  writer.write("z-pos",layer);
  switch (gradient_direction) {
    case HORIZONTAL:        writer.write("direction", "horizontal"       , false); break;
    case VERTICAL_SECTOR:   writer.write("direction", "vertical_sector"  , false); break;
    case HORIZONTAL_SECTOR: writer.write("direction", "horizontal_sector", false); break;
    case VERTICAL: break;
  }
  if(gradient_direction == HORIZONTAL || gradient_direction == HORIZONTAL_SECTOR) {
    writer.write("left_color" , gradient_top.toVector(false));
    writer.write("right_color", gradient_bottom.toVector(false));
  } else {
    writer.write("top_color"   , gradient_top.toVector(false));
    writer.write("bottom_color", gradient_bottom.toVector(false));
  }
}

ObjectSettings
Gradient::get_settings() {
  ObjectSettings result(_("Gradient"));
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Name"), &name));

  ObjectOption doo(MN_STRINGSELECT, _("Direction"), &gradient_direction);
  doo.select.push_back(_("vertical"));
  doo.select.push_back(_("horizontal"));
  doo.select.push_back(_("vertical sector"));
  doo.select.push_back(_("horizontal sector"));
  result.options.push_back(doo);

  return result;
}

Gradient::~Gradient()
{
}

void
Gradient::update(float)
{
}

void
Gradient::set_gradient(Color top, Color bottom)
{
  gradient_top = top;
  gradient_bottom = bottom;

  if (gradient_top.red > 1.0 || gradient_top.green > 1.0 ||
      gradient_top.blue > 1.0 || gradient_top.alpha > 1.0)
  {
    log_warning << "top gradient color has values above 1.0" << std::endl;
  }

  if (gradient_bottom.red > 1.0 || gradient_bottom.green > 1.0 ||
       gradient_bottom.blue > 1.0 || gradient_bottom.alpha > 1.0)
  {
    log_warning << "bottom gradient color has values above 1.0" << std::endl;
  }
}

void
Gradient::draw(DrawingContext& context)
{
  if(gradient_direction != HORIZONTAL && gradient_direction != VERTICAL)
  {
      auto current_sector = Sector::current();
      auto camera_translation = current_sector->camera->get_translation();
      auto sector_width = current_sector->get_width();
      auto sector_height = current_sector->get_height();
      gradient_region = Rectf(-camera_translation.x, -camera_translation.y, sector_width, sector_height);
  }

  context.push_transform();
  context.set_translation(Vector(0, 0));
  context.draw_gradient(gradient_top, gradient_bottom, layer, gradient_direction, gradient_region);
  context.pop_transform();
}

/* EOF */
