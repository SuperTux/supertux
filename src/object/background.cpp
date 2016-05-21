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

#include <iostream>
#include <math.h>
#include <stdexcept>

#include "math/sizef.hpp"
#include "scripting/background.hpp"
#include "scripting/squirrel_util.hpp"
#include "supertux/globals.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"
#include "util/log.hpp"
#include "util/reader.hpp"
#include "util/reader_mapping.hpp"

Background::Background() :
  alignment(NO_ALIGNMENT),
  layer(LAYER_BACKGROUND0),
  imagefile_top(),
  imagefile(),
  imagefile_bottom(),
  pos(),
  speed(),
  speed_y(),
  scroll_speed(),
  scroll_offset(),
  image_top(),
  image(),
  image_bottom(),
  has_pos_x(false),
  has_pos_y(false)
{
}

Background::Background(const ReaderMapping& reader) :
  alignment(NO_ALIGNMENT),
  layer(LAYER_BACKGROUND0),
  imagefile_top(),
  imagefile(),
  imagefile_bottom(),
  pos(),
  speed(),
  speed_y(),
  scroll_speed(),
  scroll_offset(),
  image_top(),
  image(),
  image_bottom(),
  has_pos_x(false),
  has_pos_y(false)
{
  // read position, defaults to (0,0)
  float px = 0;
  float py = 0;
  has_pos_x = reader.get("x", px);
  has_pos_y = reader.get("y", py);
  this->pos = Vector(px,py);

  if (!reader.get("name", name)) name = "";

  speed = 1.0;
  speed_y = 1.0;

  std::string alignment_str;
  if (reader.get("alignment", alignment_str))
  {
    if (alignment_str == "left")
    {
      alignment = LEFT_ALIGNMENT;
    }
    else if (alignment_str == "right")
    {
      alignment = RIGHT_ALIGNMENT;
    }
    else if (alignment_str == "top")
    {
      alignment = TOP_ALIGNMENT;
    }
    else if (alignment_str == "bottom")
    {
      alignment = BOTTOM_ALIGNMENT;
    }
    else if (alignment_str == "none")
    {
      alignment = NO_ALIGNMENT;
    }
    else
    {
      log_warning << "Background: invalid alignment: '" << alignment_str << "'" << std::endl;
      alignment = NO_ALIGNMENT;
    }
  }

  if (!reader.get("scroll-offset-x", scroll_offset.x)) scroll_offset.x = 0;
  if (!reader.get("scroll-offset-y", scroll_offset.y)) scroll_offset.y = 0;

  if (!reader.get("scroll-speed-x", scroll_speed.x)) scroll_speed.x = 0.5;
  if (!reader.get("scroll-speed-y", scroll_speed.y)) scroll_speed.y = 0.5;

  layer = reader_get_layer (reader, /* default = */ LAYER_BACKGROUND0);

  if (!reader.get("image", imagefile)) imagefile = "images/background/transparent_up.png";
  if (!reader.get("speed", speed)) speed = 0.5;

  set_image(imagefile, speed);
  if (!reader.get("speed-y", speed_y))
  {
    speed_y = speed;
  }

  if (reader.get("image-top", imagefile_top)) {
    image_top = Surface::create(imagefile_top);
  } else {
    imagefile_top = imagefile;
  }

  if (reader.get("image-bottom", imagefile_bottom)) {
    image_bottom = Surface::create(imagefile_bottom);
  } else {
    imagefile_bottom = imagefile;
  }
}

Background::~Background()
{
}

void
Background::save(Writer& writer) {
  GameObject::save(writer);
  switch (alignment) {
    case LEFT_ALIGNMENT:   writer.write("alignment", "left",   false); break;
    case RIGHT_ALIGNMENT:  writer.write("alignment", "right",  false); break;
    case TOP_ALIGNMENT:    writer.write("alignment", "top",    false); break;
    case BOTTOM_ALIGNMENT: writer.write("alignment", "bottom", false); break;
    case NO_ALIGNMENT: break;
  }

  writer.write("scroll-offset-x", scroll_offset.x);
  writer.write("scroll-offset-y", scroll_offset.y);
  writer.write("scroll-speed-x",  scroll_speed.x);
  writer.write("scroll-speed-y",  scroll_speed.y);
  writer.write("speed", speed);
  if (speed_y != speed){
    writer.write("speed_y", speed_y);
  }

  writer.write("image", imagefile, false);
  if (imagefile_top != "") {
    writer.write("image-top", imagefile_top);
  }
  if (imagefile_bottom != "") {
    writer.write("image-bottom", imagefile_bottom);
  }
}

ObjectSettings
Background::get_settings() {
  ObjectSettings result = GameObject::get_settings();
  result.options.push_back( ObjectOption(MN_INTFIELD, _("Z-pos"), &layer));
  result.options.push_back( ObjectOption(MN_NUMFIELD, _("Scroll offset x"), &scroll_offset.x));
  result.options.push_back( ObjectOption(MN_NUMFIELD, _("Scroll offset y"), &scroll_offset.y));
  result.options.push_back( ObjectOption(MN_NUMFIELD, _("Scroll speed x"), &scroll_speed.x));
  result.options.push_back( ObjectOption(MN_NUMFIELD, _("Scroll speed y"), &scroll_speed.y));
  result.options.push_back( ObjectOption(MN_NUMFIELD, _("Speed x"), &speed));
  result.options.push_back( ObjectOption(MN_NUMFIELD, _("Speed y"), &speed_y));

  ObjectOption img(MN_FILE, _("Top image"), &imagefile_top);
  img.select.push_back(".png");
  img.select.push_back(".jpg");
  img.select.push_back(".gif");
  img.select.push_back(".bmp");
  result.options.push_back(img);
  ObjectOption img2(MN_FILE, _("Image"), &imagefile);
  img2.select = img.select;
  ObjectOption img3(MN_FILE, _("Bottom image"), &imagefile_bottom);
  img3.select = img.select;
  result.options.push_back(img2);
  result.options.push_back(img3);

  result.options.push_back( ObjectOption(MN_REMOVE, "", NULL));
  return result;
}

void
Background::after_editor_set()
{
  image_top = Surface::create(imagefile_top);
  image = Surface::create(imagefile);
  image_bottom = Surface::create(imagefile_bottom);
}

void
Background::update(float delta)
{
  scroll_offset += scroll_speed * delta;
}

void
Background::set_image(const std::string& name_)
{
  this->imagefile = name_;
  image = Surface::create(name_);
  imagefile = name_;
}

void
Background::set_image(const std::string& name_, float speed_)
{
  this->speed = speed_;
  set_image(name_);
}

void
Background::set_images(const std::string& name_top_, const std::string& name_middle_,
                       const std::string& name_bottom_)
{
  image_top = Surface::create(name_top_);
  imagefile_top = name_top_;

  image = Surface::create(name_middle_);
  imagefile = name_middle_;

  image_bottom = Surface::create(name_bottom_);
  imagefile_bottom = name_bottom_;
}

void
Background::set_speed(float speed_)
{
  speed = speed_;
}

void
Background::draw_image(DrawingContext& context, const Vector& pos_)
{
  Sizef level(Sector::current()->get_width(), Sector::current()->get_height());
  Sizef screen(SCREEN_WIDTH, SCREEN_HEIGHT);
  Sizef parallax_image_size = (1.0f - speed) * screen + level * speed;
  Rectf cliprect = context.get_cliprect();

  int start_x = static_cast<int>(floorf((cliprect.get_left()  - (pos_.x - image->get_width() /2.0f)) / image->get_width()));
  int end_x   = static_cast<int>(ceilf((cliprect.get_right()  - (pos_.x + image->get_width() /2.0f)) / image->get_width()))+1;
  int start_y = static_cast<int>(floorf((cliprect.get_top()   - (pos_.y - image->get_height()/2.0f)) / image->get_height()));
  int end_y   = static_cast<int>(ceilf((cliprect.get_bottom() - (pos_.y + image->get_height()/2.0f)) / image->get_height()))+1;

  switch(alignment)
  {
    case LEFT_ALIGNMENT:
      for(int y = start_y; y < end_y; ++y)
      {
        Vector p(pos_.x - parallax_image_size.width / 2.0f,
                 pos_.y + y * image->get_height()  - image->get_height() / 2.0f);
        context.draw_surface(image, p, layer);
      }
      break;

    case RIGHT_ALIGNMENT:
      for(int y = start_y; y < end_y; ++y)
      {
        Vector p(pos_.x + parallax_image_size.width / 2.0f - image->get_width(),
                 pos_.y + y * image->get_height() - image->get_height() / 2.0f);
        context.draw_surface(image, p, layer);
      }
      break;

    case TOP_ALIGNMENT:
      for(int x = start_x; x < end_x; ++x)
      {
        Vector p(pos_.x + x * image->get_width() - image->get_width() / 2.0f,
                 pos_.y - parallax_image_size.height / 2.0f);
        context.draw_surface(image, p, layer);
      }
      break;

    case BOTTOM_ALIGNMENT:
      for(int x = start_x; x < end_x; ++x)
      {
        Vector p(pos_.x + x * image->get_width()  - image->get_width() / 2.0f,
                 pos_.y - image->get_height() + parallax_image_size.height / 2.0f);
        context.draw_surface(image, p, layer);
      }
      break;

    case NO_ALIGNMENT:
      for(int y = start_y; y < end_y; ++y)
        for(int x = start_x; x < end_x; ++x)
        {
          Vector p(pos_.x + x * image->get_width()  - image->get_width()/2,
                   pos_.y + y * image->get_height() - image->get_height()/2);

          if (image_top.get() != NULL && (y < 0))
          {
            context.draw_surface(image_top, p, layer);
          }
          else if (image_bottom.get() != NULL && (y > 0))
          {
            context.draw_surface(image_bottom, p, layer);
          }
          else
          {
            context.draw_surface(image, p, layer);
          }
        }
      break;
  }
}

void
Background::draw(DrawingContext& context)
{
  if(image.get() == NULL)
    return;

  Sizef level_size(Sector::current()->get_width(),
                   Sector::current()->get_height());
  Sizef screen(SCREEN_WIDTH, SCREEN_HEIGHT);
  Sizef translation_range = level_size - screen;
  Vector center_offset(context.get_translation().x - translation_range.width  / 2.0f,
                       context.get_translation().y - translation_range.height / 2.0f);

  float px = has_pos_x ? pos.x : level_size.width/2;
  float py = has_pos_y ? pos.y : level_size.height/2;
  draw_image(context, Vector(px, py) + center_offset * (1.0f - speed));
}

void
Background::expose(HSQUIRRELVM vm, SQInteger table_idx)
{
  if (name.empty())
    return;

  scripting::Background* _this = new scripting::Background(this);
  expose_object(vm, table_idx, _this, name, true);
}

void
Background::unexpose(HSQUIRRELVM vm, SQInteger table_idx)
{
  if (name.empty())
    return;

  scripting::unexpose_object(vm, table_idx, name);
}

/* EOF */
