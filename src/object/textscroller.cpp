//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//                2018 Ingo Ruhnke <grumbel@gmail.com>
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

#include "object/textscroller.hpp"

#include <boost/optional.hpp>
#include <sexp/value.hpp>

#include "control/input_manager.hpp"
#include "supertux/globals.hpp"
#include "supertux/fadetoblack.hpp"
#include "supertux/info_box_line.hpp"
#include "supertux/screen_manager.hpp"
#include "util/log.hpp"
#include "util/reader.hpp"
#include "util/reader_collection.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"
#include "video/drawing_context.hpp"
#include "video/video_system.hpp"
#include "video/viewport.hpp"

namespace {

const float LEFT_BORDER = 0;
const float DEFAULT_SPEED = 60;

} // namespace

TextScroller::TextScroller(const ReaderMapping& mapping) :
  controller(),
  m_filename(),
  m_lines(),
  m_scroll(),
  m_speed(DEFAULT_SPEED),
  m_finished(false),
  m_fading(false)
{
  if (!mapping.get("file", m_filename))
  {
    log_warning << mapping.get_doc().get_filename() << "'file' tag missing" << std::endl;
  }
  else
  {
    parse_file(m_filename);
  }

  mapping.get("speed", m_speed);
}

TextScroller::TextScroller(const ReaderObject& root) :
  controller(),
  m_filename(),
  m_lines(),
  m_scroll(),
  m_speed(DEFAULT_SPEED),
  m_finished(false),
  m_fading(false)
{
  parse_root(root);
}

void
TextScroller::parse_file(const std::string& filename)
{
  register_translation_directory(filename);
  auto doc = ReaderDocument::from_file(filename);
  auto root = doc.get_root();
  parse_root(root);
}

void
TextScroller::parse_root(const ReaderObject& root)
{
  if (root.get_name() != "supertux-text")
  {
    throw std::runtime_error("File isn't a supertux-text file");
  }
  else
  {
    auto mapping = root.get_mapping();

    int version = 1;
    mapping.get("version", version);
    if (version == 1)
    {
      log_info << "[" << mapping.get_doc().get_filename() << "] Text uses old format: version 1" << std::endl;

      std::string text;
      if (!mapping.get("text", text)) {
        throw std::runtime_error("File doesn't contain a text field");
      }

      // Split text string lines into a vector
      m_lines = InfoBoxLine::split(text, static_cast<float>(SCREEN_WIDTH) - 2.0f * LEFT_BORDER);
    }
    else if (version == 2)
    {
      boost::optional<ReaderCollection> content_collection;
      if (!mapping.get("content", content_collection)) {
        throw std::runtime_error("File doesn't contain content");
      } else {
        parse_content(*content_collection);
      }
    }
  }
}

void
TextScroller::parse_content(const ReaderCollection& collection)
{
  for (const auto& item : collection.get_objects())
  {
    if (item.get_name() == "image")
    {
      std::string image_file = item.get_sexp().as_array()[1].as_string();
      m_lines.emplace_back(new InfoBoxLine('!', image_file));
    }
    else if (item.get_name() == "person")
    {
      bool simple;
      std::string name, info, image_file;

      if (!item.get_mapping().get("simple", simple)) {
        simple = false;
      }

      if (simple) {
        if (!item.get_mapping().get("name", name) || !item.get_mapping().get("info", info)) {
          throw std::runtime_error("Simple entry requires both name and info specified");
        }

        if (item.get_mapping().get("image", image_file)) {
          log_warning << "[" << collection.get_doc().get_filename() << "] Simple person entry shouldn't specify images" << std::endl;
        }

        m_lines.emplace_back(new InfoBoxLine(' ', name + " (" + info + ")")); // NOLINT
      } else {
        if (item.get_mapping().get("name", name)) {
          m_lines.emplace_back(new InfoBoxLine('\t', name));
        }

        if (item.get_mapping().get("image", image_file) && !simple) {
          m_lines.emplace_back(new InfoBoxLine('!', image_file));
        }

        if (item.get_mapping().get("info", info)) {
          m_lines.emplace_back(new InfoBoxLine(' ', info));
        }
      }
    }
    else if (item.get_name() == "blank")
    {
      // Empty line
      m_lines.emplace_back(new InfoBoxLine('\t', ""));
    }
    else if (item.get_name() == "text")
    {
      std::string type, string;

      if (!item.get_mapping().get("type", type)) {
        type = "normal";
      }

      if (!item.get_mapping().get("string", string)) {
        throw std::runtime_error("Text entry requires a string");
      }

      if (type == "normal")
        m_lines.emplace_back(new InfoBoxLine('\t', string));
      else if (type == "normal-left")
        m_lines.emplace_back(new InfoBoxLine('#', string));
      else if (type == "small")
        m_lines.emplace_back(new InfoBoxLine(' ', string));
      else if (type == "heading")
        m_lines.emplace_back(new InfoBoxLine('-', string));
      else if (type == "reference")
        m_lines.emplace_back(new InfoBoxLine('*', string));
      else {
        log_warning << "[" << item.get_doc().get_filename() << "] Unknown text type '" << type << "'" << std::endl;
        m_lines.emplace_back(new InfoBoxLine('\t', string));
      }
    }
    else
    {
      log_warning << "[" << item.get_doc().get_filename() << "] Unknown token '" << item.get_name() << "'" << std::endl;
    }
  }
}

void
TextScroller::draw(DrawingContext& context)
{
  context.push_transform();
  context.set_translation(Vector(0, 0));

  const float ctx_w = static_cast<float>(context.get_width());
  const float ctx_h = static_cast<float>(context.get_height());

  float y = floorf(ctx_h - m_scroll);

  { // draw text
    for (const auto& line : m_lines)
    {
      if (y + line->get_height() >= 0 && ctx_h - y >= 0) {
        line->draw(context, Rectf(LEFT_BORDER, y, ctx_w - 2*LEFT_BORDER, y), LAYER_GUI);
      }

      y += floorf(line->get_height());
    }
  }

  context.pop_transform();

  // close when done
  if (y < 0)
  {
    m_finished = true;
  }
}

void
TextScroller::update(float dt_sec)
{
  m_scroll += m_speed * dt_sec;

  if (m_scroll < 0)
    m_scroll = 0;

  if (controller.pressed(Control::START) ||
      controller.pressed(Control::ESCAPE)) {
    ScreenManager::current()->pop_screen(std::unique_ptr<ScreenFade>(new FadeToBlack(FadeToBlack::FADEOUT, 0.25)));
  }
  
  { // close when done
    if (m_finished && !m_fading)
    {
	  m_fading = true;
      ScreenManager::current()->pop_screen(std::unique_ptr<ScreenFade>(new FadeToBlack(FadeToBlack::FADEOUT, 0.25)));
    }
  }
}

void
TextScroller::set_speed(float speed)
{
  m_speed = speed;
}

void
TextScroller::scroll(float offset)
{
  m_scroll += offset;
  if (m_scroll < 0.0f)
  {
    m_scroll = 0.0f;
  }
}

ObjectSettings
TextScroller::get_settings()
{
  ObjectSettings result = GameObject::get_settings();

  result.add_float(_("Speed"), &m_speed, "speed", DEFAULT_SPEED);
  result.add_file(_("File"), &m_filename, "file");

  return result;
}

/* EOF */
