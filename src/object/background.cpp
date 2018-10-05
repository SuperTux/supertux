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

#include "editor/editor.hpp"
#include "supertux/globals.hpp"
#include "supertux/sector.hpp"
#include "util/reader.hpp"
#include "util/reader_mapping.hpp"
#include "util/writer.hpp"
#include "video/drawing_context.hpp"
#include "video/surface.hpp"

Background::Background() :
  ExposedObject<Background, scripting::Background>(this),
  m_alignment(NO_ALIGNMENT),
  m_fill(false),
  m_layer(LAYER_BACKGROUND0),
  m_imagefile_top(),
  m_imagefile(),
  m_imagefile_bottom(),
  m_pos(),
  m_speed(),
  m_speed_y(),
  m_scroll_speed(),
  m_scroll_offset(),
  m_image_top(),
  m_image(),
  m_image_bottom(),
  m_has_pos_x(false),
  m_has_pos_y(false),
  m_blend(),
  m_target(DrawingTarget::COLORMAP)
{
}

Background::Background(const ReaderMapping& reader) :
  GameObject(reader),
  ExposedObject<Background, scripting::Background>(this),
  m_alignment(NO_ALIGNMENT),
  m_fill(false),
  m_layer(LAYER_BACKGROUND0),
  m_imagefile_top(),
  m_imagefile(),
  m_imagefile_bottom(),
  m_pos(),
  m_speed(),
  m_speed_y(),
  m_scroll_speed(),
  m_scroll_offset(),
  m_image_top(),
  m_image(),
  m_image_bottom(),
  m_has_pos_x(false),
  m_has_pos_y(false),
  m_blend(),
  m_target(DrawingTarget::COLORMAP)
{
  // read position, defaults to (0,0)
  float px = 0;
  float py = 0;
  m_has_pos_x = reader.get("x", px);
  m_has_pos_y = reader.get("y", py);
  m_pos = Vector(px,py);

  m_speed = 1.0;
  m_speed_y = 1.0;

  reader.get("fill", m_fill);

  std::string alignment_str;
  if (reader.get("alignment", alignment_str))
  {
    if (alignment_str == "left")
    {
      m_alignment = LEFT_ALIGNMENT;
    }
    else if (alignment_str == "right")
    {
      m_alignment = RIGHT_ALIGNMENT;
    }
    else if (alignment_str == "top")
    {
      m_alignment = TOP_ALIGNMENT;
    }
    else if (alignment_str == "bottom")
    {
      m_alignment = BOTTOM_ALIGNMENT;
    }
    else if (alignment_str == "none")
    {
      m_alignment = NO_ALIGNMENT;
    }
    else
    {
      log_warning << "Background: invalid alignment: '" << alignment_str << "'" << std::endl;
      m_alignment = NO_ALIGNMENT;
    }
  }

  reader.get("scroll-offset-x", m_scroll_offset.x, 0.0f);
  reader.get("scroll-offset-y", m_scroll_offset.y, 0.0f);

  reader.get("scroll-speed-x", m_scroll_speed.x, 0.5f);
  reader.get("scroll-speed-y", m_scroll_speed.y, 0.5f);

  m_layer = reader_get_layer (reader, /* default = */ LAYER_BACKGROUND0);

  reader.get("image", m_imagefile, "images/background/transparent_up.png");
  reader.get("speed", m_speed, 0.5f);

  set_image(m_imagefile, m_speed);
  reader.get("speed-y", m_speed_y, m_speed);

  if (reader.get("image-top", m_imagefile_top)) {
    m_image_top = Surface::from_file(m_imagefile_top);
  } else {
    m_imagefile_top = m_imagefile;
  }

  if (reader.get("image-bottom", m_imagefile_bottom)) {
    m_image_bottom = Surface::from_file(m_imagefile_bottom);
  } else {
    m_imagefile_bottom = m_imagefile;
  }

  reader.get_custom("blend", m_blend, Blend::from_string);
  reader.get_custom("target", m_target, DrawingTarget_from_string);
}

Background::~Background()
{
}

void
Background::save(Writer& writer) {
  GameObject::save(writer);
  switch (m_alignment) {
    case LEFT_ALIGNMENT:   writer.write("alignment", "left",   false); break;
    case RIGHT_ALIGNMENT:  writer.write("alignment", "right",  false); break;
    case TOP_ALIGNMENT:    writer.write("alignment", "top",    false); break;
    case BOTTOM_ALIGNMENT: writer.write("alignment", "bottom", false); break;
    case NO_ALIGNMENT: break;
  }

  if (m_speed_y != m_speed) {
    writer.write("speed_y", m_speed_y);
  }
}

ObjectSettings
Background::get_settings() {
  ObjectSettings result = GameObject::get_settings();
  result.options.push_back( ObjectOption(MN_INTFIELD, _("Z-pos"), &m_layer, "z-pos"));
  ObjectOption align(MN_STRINGSELECT, _("Alignment"), &m_alignment);
  align.select.push_back(_("none"));
  align.select.push_back(_("left"));
  align.select.push_back(_("right"));
  align.select.push_back(_("top"));
  align.select.push_back(_("bottom"));
  result.options.push_back(align);
  result.options.push_back( ObjectOption(MN_NUMFIELD, _("Scroll offset x"),
                                         &m_scroll_offset.x, "scroll-offset-x"));
  result.options.push_back( ObjectOption(MN_NUMFIELD, _("Scroll offset y"),
                                         &m_scroll_offset.y, "scroll-offset-y"));
  result.options.push_back( ObjectOption(MN_NUMFIELD, _("Scroll speed x"),
                                         &m_scroll_speed.x, "scroll-speed-x"));
  result.options.push_back( ObjectOption(MN_NUMFIELD, _("Scroll speed y"),
                                         &m_scroll_speed.y, "scroll-speed-y"));
  result.options.push_back( ObjectOption(MN_NUMFIELD, _("Speed x"), &m_speed, "speed"));
  result.options.push_back( ObjectOption(MN_NUMFIELD, _("Speed y"), &m_speed_y));

  ObjectOption img(MN_FILE, _("Top image"), &m_imagefile_top, "image-top", (OPTION_VISIBLE));
  img.select.push_back(".png");
  img.select.push_back(".jpg");
  img.select.push_back(".gif");
  img.select.push_back(".bmp");
  result.options.push_back(img);
  ObjectOption img2(MN_FILE, _("Image"), &m_imagefile, "image");
  img2.select = img.select;
  ObjectOption img3(MN_FILE, _("Bottom image"), &m_imagefile_bottom, "image-bottom", (OPTION_VISIBLE));
  img3.select = img.select;
  result.options.push_back(img2);
  result.options.push_back(img3);

  result.options.push_back( ObjectOption(MN_REMOVE, "", NULL));
  return result;
}

void
Background::after_editor_set()
{
  m_image_top = Surface::from_file(m_imagefile_top);
  m_image = Surface::from_file(m_imagefile);
  m_image_bottom = Surface::from_file(m_imagefile_bottom);
}

void
Background::update(float delta)
{
  m_scroll_offset += m_scroll_speed * delta;
}

void
Background::set_image(const std::string& name_)
{
  m_imagefile = name_;
  m_image = Surface::from_file(name_);
  m_imagefile = name_;
}

void
Background::set_image(const std::string& name_, float speed_)
{
  m_speed = speed_;
  set_image(name_);
}

void
Background::set_images(const std::string& name_top_, const std::string& name_middle_,
                       const std::string& name_bottom_)
{
  m_image_top = Surface::from_file(name_top_);
  m_imagefile_top = name_top_;

  m_image = Surface::from_file(name_middle_);
  m_imagefile = name_middle_;

  m_image_bottom = Surface::from_file(name_bottom_);
  m_imagefile_bottom = name_bottom_;
}

void
Background::set_speed(float speed_)
{
  m_speed = speed_;
}

void
Background::draw_image(DrawingContext& context, const Vector& pos__)
{
  const Vector pos_ = pos__.to_int_vec();

  const Sizef level(Sector::current()->get_width(), Sector::current()->get_height());
  const Sizef screen(static_cast<float>(context.get_width()),
                     static_cast<float>(context.get_height()));
  const Sizef parallax_image_size = (1.0f - m_speed) * screen + level * m_speed;

  const Rectf cliprect = context.get_cliprect();
  const float img_w = static_cast<float>(m_image->get_width());
  const float img_h = static_cast<float>(m_image->get_height());

  const float img_w_2 = floorf(img_w / 2.0f);
  const float img_h_2 = floorf(img_h / 2.0f);

  const int start_x = static_cast<int>(floorf((cliprect.get_left() - (pos_.x - img_w /2.0f)) / img_w));
  const int end_x   = static_cast<int>(ceilf((cliprect.get_right() - (pos_.x + img_w /2.0f)) / img_w)) + 1;
  const int start_y = static_cast<int>(floorf((cliprect.get_top() - (pos_.y - img_h/2.0f)) / img_h));
  const int end_y   = static_cast<int>(ceilf((cliprect.get_bottom() - (pos_.y + img_h/2.0f)) / img_h)) + 1;

  Canvas& canvas = context.get_canvas(m_target);

  if (m_fill)
  {
    Rectf dstrect(Vector(pos_.x - static_cast<float>(context.get_width()) / 2.0f,
                         pos_.y - static_cast<float>(context.get_height()) / 2.0f),
                  Sizef(static_cast<float>(context.get_width()),
                        static_cast<float>(context.get_height())));
    canvas.draw_surface_scaled(m_image, dstrect, m_layer);
  }
  else
  {
    switch(m_alignment)
    {
      case LEFT_ALIGNMENT:
        for(int y = start_y; y < end_y; ++y)
        {
          Vector p(pos_.x - parallax_image_size.width / 2.0f,
                   pos_.y + static_cast<float>(y) * img_h - img_h_2);
          canvas.draw_surface(m_image, p, m_layer);
        }
        break;

      case RIGHT_ALIGNMENT:
        for(int y = start_y; y < end_y; ++y)
        {
          Vector p(pos_.x + parallax_image_size.width / 2.0f - img_w,
                   pos_.y + static_cast<float>(y) * img_h - img_h_2);
          canvas.draw_surface(m_image, p, m_layer);
        }
        break;

      case TOP_ALIGNMENT:
        for(int x = start_x; x < end_x; ++x)
        {
          Vector p(pos_.x + static_cast<float>(x) * img_w - img_w_2,
                   pos_.y - parallax_image_size.height / 2.0f);
          canvas.draw_surface(m_image, p, m_layer);
        }
        break;

      case BOTTOM_ALIGNMENT:
        for(int x = start_x; x < end_x; ++x)
        {
          Vector p(pos_.x + static_cast<float>(x) * img_w - img_w_2,
                   pos_.y - img_h + parallax_image_size.height / 2.0f);
          canvas.draw_surface(m_image, p, m_layer);
        }
        break;

      case NO_ALIGNMENT:
        for(int y = start_y; y < end_y; ++y)
          for(int x = start_x; x < end_x; ++x)
          {
            Vector p(pos_.x + static_cast<float>(x) * img_w - img_w_2,
                     pos_.y + static_cast<float>(y) * img_h - img_h_2);

            if (m_image_top.get() != NULL && (y < 0))
            {
              canvas.draw_surface(m_image_top, p, m_layer);
            }
            else if (m_image_bottom.get() != NULL && (y > 0))
            {
              canvas.draw_surface(m_image_bottom, p, m_layer);
            }
            else
            {
              canvas.draw_surface(m_image, p, m_layer);
            }
          }
        break;
    }
  }
}

void
Background::draw(DrawingContext& context)
{
  if(Editor::is_active() && !EditorInputCenter::render_background)
    return;

  if(m_image.get() == NULL)
    return;

  Sizef level_size(Sector::current()->get_width(),
                   Sector::current()->get_height());
  Sizef screen(static_cast<float>(context.get_width()),
               static_cast<float>(context.get_height()));
  Sizef translation_range = level_size - screen;
  Vector center_offset(context.get_translation().x - translation_range.width  / 2.0f,
                       context.get_translation().y - translation_range.height / 2.0f);

  float px = m_has_pos_x ? m_pos.x : level_size.width/2;
  float py = m_has_pos_y ? m_pos.y : level_size.height/2;
  draw_image(context, Vector(px, py) + center_offset * (1.0f - m_speed));
}

/* EOF */
