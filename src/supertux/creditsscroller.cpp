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

#include "supertux/creditsscroller.hpp"

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

CreditsScroller::CreditsScroller(const std::string& filename) :
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

  std::string background_file;

  try {
    register_translation_directory(filename);
    auto doc = ReaderDocument::parse(filename);
    auto root = doc.get_root();

    if(root.get_name() != "supertux-credits") {
      throw std::runtime_error("File isn't a supertux-credits file");
    } else {
      auto credits = root.get_mapping();

      if(!credits.get("background", background_file)) {
        throw std::runtime_error("File doesn't contain a background file");
      }

      ReaderMapping content;
      if(!credits.get("content", content)) {
        throw std::runtime_error("No content specified in credits file");
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
                log_warning << "Image specified in a simple person entry (" << filename << ")" << std::endl;
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

            if(type == "normal")
              lines.emplace_back(new InfoBoxLine('\t', string));
            else if(type == "normal-left")
              lines.emplace_back(new InfoBoxLine('#', string));
            else if(type == "small")
              lines.emplace_back(new InfoBoxLine(' ', string));
            else if(type == "heading")
              lines.emplace_back(new InfoBoxLine('-', string));
            else if(type == "reference")
              lines.emplace_back(new InfoBoxLine('*', string));
            else {
              log_warning << "Unknown text type '" << iter.get_key() << "'in credits file (" << filename << ")" << std::endl;
              lines.emplace_back(new InfoBoxLine('\t', string));
            }
          } else {
            log_warning << "Unknown token '" << iter.get_key() << "'in credits file (" << filename << ")" << std::endl;
          }
        }
      }

      credits.get("speed", defaultspeed);
      credits.get("music", music);
    }
  } catch(std::exception& e) {
    std::ostringstream msg;
    msg << "Couldn't load file '" << filename << "': " << e.what() << std::endl;
    throw std::runtime_error(msg.str());
  }

  // load background image
  background = Surface::create("images/background/" + background_file);

  scroll = 0;
  fading = false;
}

CreditsScroller::~CreditsScroller()
{
}

void
CreditsScroller::setup()
{
  SoundManager::current()->play_music(music);
  if(g_config->transitions_enabled)
  {
    ScreenManager::current()->set_screen_fade(std::unique_ptr<ScreenFade>(new FadeIn(0.5)));
  }
}

void
CreditsScroller::update(float elapsed_time)
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
    if(g_config->transitions_enabled)
    {
      ScreenManager::current()->pop_screen(std::unique_ptr<ScreenFade>(new FadeOut(0.5)));
    }
    else
    {
      ScreenManager::current()->pop_screen();
    }
  }

  scroll += speed * elapsed_time;

  if(scroll < 0)
    scroll = 0;
}

void
CreditsScroller::draw(DrawingContext& context)
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
