//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//  Copyright (C) 2016 M. Teufel <mteufel@urandom.eu.org>
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

#include "supertux/textscroller.hpp"

#include "audio/sound_manager.hpp"
#include "control/input_manager.hpp"
#include "supertux/fadein.hpp"
#include "supertux/fadeout.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/info_box_line.hpp"
#include "supertux/globals.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/resources.hpp"
#include "util/reader.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"
#include "video/drawing_context.hpp"

#include <sstream>
#include <stdexcept>

static const float DEFAULT_SPEED = 20;
static const float LEFT_BORDER = 50;
static const float SCROLL = 60;

TextScroller::TextScroller(const std::string& filename) :
  defaultspeed(),
  speed(),
  music(),
  background(),
  lines(),
  scroll(),
  fading()
{
  defaultspeed = DEFAULT_SPEED;
  speed = defaultspeed;

  std::string text;
  std::string background_file;

  try {
    register_translation_directory(filename);
    auto doc = ReaderDocument::parse(filename);
    auto root = doc.get_root();

    if(root.get_name() != "supertux-text") {
      throw std::runtime_error("File isn't a supertux-text file");
    } else {
      auto text_lisp = root.get_mapping();

      int version = 1;
      text_lisp.get("version", version);
      if (version == 1) {
        log_info << "[" << filename << "] Text uses old format: version 1" << std::endl;

        if(!text_lisp.get("text", text)) {
          throw std::runtime_error("File doesn't contain a text field");
        }

        // Split text string lines into a vector
        lines = InfoBoxLine::split(text, SCREEN_WIDTH - 2*LEFT_BORDER);
      } else if (version == 2) {
        ReaderMapping content;
        if (!text_lisp.get("content", content)) {
          throw std::runtime_error("File doesn't contain content");
        } else {
          auto iter = content.get_iter();
          while (iter.next()) {
            if (iter.get_key() == "image") {
              std::string image_file;
              iter.get(image_file);
              lines.emplace_back(new InfoBoxLine('!', image_file));
            } else if (iter.get_key() == "person") {
              bool simple;
              std::string name, info, image_file;

              if (!iter.as_mapping().get("simple", simple)) {
                simple = false;
              }

              if (simple) {
                if (!iter.as_mapping().get("name", name) || !iter.as_mapping().get("info", info)) {
                  throw std::runtime_error("Simple entry requires both name and info specified");
                }

                if (iter.as_mapping().get("image", image_file)) {
                  log_warning << "[" << filename << "] Simple person entry shouldn't specify images" << std::endl;
                }

                lines.emplace_back(new InfoBoxLine(' ', name + " (" + info + ")"));
              } else {
                if (iter.as_mapping().get("name", name)) {
                  lines.emplace_back(new InfoBoxLine('\t', name));
                }

                if (iter.as_mapping().get("image", image_file) && !simple) {
                  lines.emplace_back(new InfoBoxLine('!', image_file));
                }

                if (iter.as_mapping().get("info", info)) {
                  lines.emplace_back(new InfoBoxLine(' ', info));
                }
              }
            } else if (iter.get_key() == "blank") {
              // Empty line
              lines.emplace_back(new InfoBoxLine('\t', ""));
            } else if (iter.get_key() == "text") {
              std::string type, string;

              if (!iter.as_mapping().get("type", type)) {
                type = "normal";
              }

              if (!iter.as_mapping().get("string", string)) {
                throw std::runtime_error("Text entry requires a string");
              }

              if (type == "normal")
                lines.emplace_back(new InfoBoxLine('\t', string));
              else if (type == "normal-left")
                lines.emplace_back(new InfoBoxLine('#', string));
              else if (type == "small")
                lines.emplace_back(new InfoBoxLine(' ', string));
              else if (type == "heading")
                lines.emplace_back(new InfoBoxLine('-', string));
              else if (type == "reference")
                lines.emplace_back(new InfoBoxLine('*', string));
              else {
                log_warning << "[" << filename << "] Unknown text type '" << type << "'" << std::endl;
                lines.emplace_back(new InfoBoxLine('\t', string));
              }
            } else {
              log_warning << "[" << filename << "] Unknown token '" << iter.get_key() << "'" << std::endl;
            }
          }
        }
      } else {
        throw std::runtime_error("File format version is not supported");
      }

      if (!text_lisp.get("background", background_file)) {
        throw std::runtime_error("File doesn't contain a background file");
      }

      text_lisp.get("speed", defaultspeed);
      text_lisp.get("music", music);
    }
  } catch (std::exception& e) {
    std::ostringstream msg;
    msg << "Couldn't load file '" << filename << "': " << e.what() << std::endl;
    throw std::runtime_error(msg.str());
  }

  // load background image
  background = Surface::create("images/background/" + background_file);

  scroll = 0;
  fading = false;
}

TextScroller::~TextScroller()
{
}

void
TextScroller::setup()
{
  SoundManager::current()->play_music(music);
  ScreenManager::current()->set_screen_fade(std::unique_ptr<ScreenFade>(new FadeIn(0.5)));
}

void
TextScroller::update(float elapsed_time)
{
  Controller* controller = InputManager::current()->get_controller();
  if(controller->hold(Controller::UP)) {
    speed = -defaultspeed*5;
  } else if(controller->hold(Controller::DOWN)) {
    speed = defaultspeed*5;
  } else {
    speed = defaultspeed;
  }
  if((controller->pressed(Controller::JUMP)
     || controller->pressed(Controller::ACTION)
     || controller->pressed(Controller::MENU_SELECT)
     )&& !(controller->pressed(Controller::UP))) // prevent skipping if jump with up is enabled
    scroll += SCROLL;
  if(controller->pressed(Controller::START) ||
     controller->pressed(Controller::ESCAPE)) {
    ScreenManager::current()->pop_screen(std::unique_ptr<ScreenFade>(new FadeOut(0.5)));
  }

  scroll += speed * elapsed_time;

  if(scroll < 0)
    scroll = 0;
}

void
TextScroller::draw(DrawingContext& context)
{
  context.draw_filled_rect(Vector(0, 0), Vector(SCREEN_WIDTH, SCREEN_HEIGHT),
                           Color(0.6f, 0.7f, 0.8f, 0.5f), 0);
  context.draw_surface_part(background, Rectf(0, 0, background->get_width(), background->get_height()),
                            Rectf(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), 0);


  float y = SCREEN_HEIGHT - scroll;
  for (auto& line : lines) {
    if (y + line->get_height() >= 0 && SCREEN_HEIGHT - y >= 0) {
      line->draw(context, Rectf(LEFT_BORDER, y, SCREEN_WIDTH - 2*LEFT_BORDER, y), LAYER_GUI);
    }

    y += line->get_height();
  }

  if(y < 0 && !fading ) {
    fading = true;
    ScreenManager::current()->pop_screen(std::unique_ptr<ScreenFade>(new FadeOut(0.5)));
  }
}

/* EOF */
