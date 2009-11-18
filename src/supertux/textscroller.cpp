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

#include "supertux/textscroller.hpp"

#include "audio/sound_manager.hpp"
#include "control/joystickkeyboardcontroller.hpp"
#include "lisp/parser.hpp"
#include "supertux/fadeout.hpp"
#include "supertux/info_box_line.hpp"
#include "supertux/globals.hpp"
#include "supertux/mainloop.hpp"
#include "supertux/resources.hpp"
#include "util/reader.hpp"
#include "video/drawing_context.hpp"

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

  lisp::Parser parser;
  try {
    const lisp::Lisp* root = parser.parse(filename);

    const lisp::Lisp* text_lisp = root->get_lisp("supertux-text");
    if(!text_lisp)
      throw std::runtime_error("File isn't a supertux-text file");

    if(!text_lisp->get("text", text))
      throw std::runtime_error("file doesn't contain a text field");
    if(!text_lisp->get("background", background_file))
      throw std::runtime_error("file doesn't contain a background file");
    text_lisp->get("speed", defaultspeed);
    text_lisp->get("music", music);
  } catch(std::exception& e) {
    std::ostringstream msg;
    msg << "Couldn't load file '" << filename << "': " << e.what() << std::endl;
    throw std::runtime_error(msg.str());
  }

  // Split text string lines into a vector
  lines = InfoBoxLine::split(text, SCREEN_WIDTH - 2*LEFT_BORDER);

  // load background image
  background.reset(new Surface("images/background/" + background_file));

  scroll = 0;
  fading = false;
}

TextScroller::~TextScroller()
{
  for(std::vector<InfoBoxLine*>::iterator i = lines.begin(); i != lines.end(); i++) delete *i;
}

void
TextScroller::setup()
{
  sound_manager->play_music(music);
}

void
TextScroller::update(float elapsed_time)
{
  if(g_main_controller->hold(Controller::UP)) {
    speed = -defaultspeed*5;
  } else if(g_main_controller->hold(Controller::DOWN)) {
    speed = defaultspeed*5;
  } else {
    speed = defaultspeed;
  }
  if(g_main_controller->pressed(Controller::JUMP)
     || g_main_controller->pressed(Controller::ACTION)
     || g_main_controller->pressed(Controller::MENU_SELECT))
    scroll += SCROLL;
  if(g_main_controller->pressed(Controller::PAUSE_MENU)) {
    g_main_loop->exit_screen(new FadeOut(0.5));
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
  context.draw_surface(background.get(), Vector(SCREEN_WIDTH/2 - background->get_width()/2 , SCREEN_HEIGHT/2 - background->get_height()/2), 0);

  float y = SCREEN_HEIGHT - scroll;
  for(size_t i = 0; i < lines.size(); i++) {
    if (y + lines[i]->get_height() >= 0 && SCREEN_HEIGHT - y >= 0) {
      lines[i]->draw(context, Rect(LEFT_BORDER, y, SCREEN_WIDTH - 2*LEFT_BORDER, y), LAYER_GUI);
    }

    y += lines[i]->get_height();
  }

  if(y < 0 && !fading ) {
    fading = true;
    g_main_loop->exit_screen(new FadeOut(0.5));
  }
}

/* EOF */
