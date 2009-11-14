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
#include <config.h>

#include "supertux/textscroller.hpp"

#include <stdexcept>
#include "util/log.hpp"
#include "supertux/mainloop.hpp"
#include "supertux/resources.hpp"
#include "video/font.hpp"
#include "video/drawing_context.hpp"
#include "video/surface.hpp"
#include "gui/menu.hpp"
#include "lisp/parser.hpp"
#include "lisp/lisp.hpp"
#include "audio/sound_manager.hpp"
#include "supertux/main.hpp"
#include "supertux/fadeout.hpp"
#include "control/joystickkeyboardcontroller.hpp"

static const float DEFAULT_SPEED = 20;
static const float LEFT_BORDER = 50;
static const float SCROLL = 60;
static const float ITEMS_SPACE = 4;

TextScroller::TextScroller(const std::string& filename)
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
  if(main_controller->hold(Controller::UP)) {
    speed = -defaultspeed*5;
  } else if(main_controller->hold(Controller::DOWN)) {
    speed = defaultspeed*5;
  } else {
    speed = defaultspeed;
  }
  if(main_controller->pressed(Controller::JUMP)
      || main_controller->pressed(Controller::ACTION)
      || main_controller->pressed(Controller::MENU_SELECT))
    scroll += SCROLL;
  if(main_controller->pressed(Controller::PAUSE_MENU)) {
    main_loop->exit_screen(new FadeOut(0.5));
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
    main_loop->exit_screen(new FadeOut(0.5));
  }
}

InfoBox::InfoBox(const std::string& text)
  : firstline(0)
{
  // Split text string lines into a vector
  lines = InfoBoxLine::split(text, 400);

  try
  {
    // get the arrow sprites
    arrow_scrollup   = new Surface("images/engine/menu/scroll-up.png");
    arrow_scrolldown = new Surface("images/engine/menu/scroll-down.png");
  }
  catch (std::exception& e)
  {
    log_warning << "Could not load scrolling images: " << e.what() << std::endl;
    arrow_scrollup = 0;
    arrow_scrolldown = 0;
  }
}

InfoBox::~InfoBox()
{
  for(std::vector<InfoBoxLine*>::iterator i = lines.begin();
      i != lines.end(); i++)
    delete *i;
  delete arrow_scrollup;
  delete arrow_scrolldown;
}

void
InfoBox::draw(DrawingContext& context)
{
  float x1 = SCREEN_WIDTH/2-200;
  float y1 = SCREEN_HEIGHT/2-200;
  float width = 400;
  float height = 200;

  context.draw_filled_rect(Vector(x1, y1), Vector(width, height),
      Color(0.6f, 0.7f, 0.8f, 0.5f), LAYER_GUI-1);

  float y = y1;
  bool linesLeft = false;
  for(size_t i = firstline; i < lines.size(); ++i) {
    if(y >= y1 + height) {
      linesLeft = true;
      break;
    }

    lines[i]->draw(context, Rect(x1, y, x1+width, y), LAYER_GUI);
    y += lines[i]->get_height();
  }

  {
    // draw the scrolling arrows
    if (arrow_scrollup && firstline > 0)
      context.draw_surface(arrow_scrollup,
      Vector( x1 + width  - arrow_scrollup->get_width(),  // top-right corner of box
              y1), LAYER_GUI);

    if (arrow_scrolldown && linesLeft && firstline < lines.size()-1)
      context.draw_surface(arrow_scrolldown,
      Vector( x1 + width  - arrow_scrolldown->get_width(),  // bottom-light corner of box
              y1 + height - arrow_scrolldown->get_height()),
              LAYER_GUI);
  }
}

void
InfoBox::scrollup()
{
  if(firstline > 0)
    firstline--;
}

void
InfoBox::scrolldown()
{
  if(firstline < lines.size()-1)
    firstline++;
}

void
InfoBox::pageup()
{
}

void
InfoBox::pagedown()
{
}

namespace {
Font* get_font_by_format_char(char format_char) {
  switch(format_char)
  {
    case ' ':
      return small_font;
      break;
    case '-':
      return big_font;
      break;
    case '\t':
    case '*':
    case '#':
    case '!':
      return normal_font;
      break;
    default:
      return normal_font;
      log_warning << "Unknown format_char: '" << format_char << "'" << std::endl;
      break;
  }
}

Color get_color_by_format_char(char format_char) {
  switch(format_char)
  {
    case ' ':
      return TextScroller::small_color;
      break;
    case '-':
      return TextScroller::heading_color;
      break;
    case '*':
      return TextScroller::reference_color;
    case '\t':
    case '#':
    case '!':
      return TextScroller::normal_color;
      break;
    default:
      return Color(0,0,0);
      log_warning << "Unknown format_char: '" << format_char << "'" << std::endl;
      break;
  }
}

InfoBoxLine::LineType get_linetype_by_format_char(char format_char) {
  switch(format_char)
  {
    case ' ':
      return InfoBoxLine::SMALL;
      break;
    case '\t':
      return InfoBoxLine::NORMAL;
      break;
    case '-':
      return InfoBoxLine::HEADING;
      break;
    case '*':
      return InfoBoxLine::REFERENCE;
      break;
    case '#':
      return InfoBoxLine::NORMAL_LEFT;
      break;
    case '!':
      return InfoBoxLine::IMAGE;
      break;
    default:
      return InfoBoxLine::SMALL;
      log_warning << "Unknown format_char: '" << format_char << "'" << std::endl;
      break;
  }
}
}

InfoBoxLine::InfoBoxLine(char format_char, const std::string& text) : lineType(NORMAL), font(normal_font), text(text), image(0)
{
  font = get_font_by_format_char(format_char);
  lineType = get_linetype_by_format_char(format_char);
  color = get_color_by_format_char(format_char);
  if (lineType == IMAGE) image = new Surface(text);
}

InfoBoxLine::~InfoBoxLine()
{
  delete image;
}

const std::vector<InfoBoxLine*>
InfoBoxLine::split(const std::string& text, float width)
{
  std::vector<InfoBoxLine*> lines;

  std::string::size_type i = 0;
  std::string::size_type l;
  char format_char = '#';
  while(i < text.size()) {
    // take care of empty lines - represent them as blank lines of normal text
    if (text[i] == '\n') {
      lines.push_back(new InfoBoxLine('\t', ""));
      i++;
      continue;
    }

    // extract the format_char
    format_char = text[i];
    i++;
    if (i >= text.size()) break;

    // extract one line
    l = text.find("\n", i);
    if (l == std::string::npos) l=text.size();
    std::string s = text.substr(i, l-i);
    i = l+1;

    // if we are dealing with an image, just store the line
    if (format_char == '!') {
      lines.push_back(new InfoBoxLine(format_char, s));
      continue;
    }

    // append wrapped parts of line into list
    std::string overflow;
    do {
      Font* font = get_font_by_format_char(format_char);
      std::string s2 = s;
      if (font) s2 = font->wrap_to_width(s2, width, &overflow);
      lines.push_back(new InfoBoxLine(format_char, s2));
      s = overflow;
    } while (s.length() > 0);
  }

  return lines;
}

void
InfoBoxLine::draw(DrawingContext& context, const Rect& bbox, int layer)
{
  Vector position = bbox.p1;
  switch (lineType) {
    case IMAGE:
      context.draw_surface(image, Vector( (bbox.p1.x + bbox.p2.x - image->get_width()) / 2, position.y), layer);
      break;
    case NORMAL_LEFT:
      context.draw_text(font, text, Vector(position.x, position.y), ALIGN_LEFT, layer, color);
      break;
    default:
      context.draw_text(font, text, Vector((bbox.p1.x + bbox.p2.x) / 2, position.y), ALIGN_CENTER, layer, color);
      break;
  }
}

float
InfoBoxLine::get_height()
{
  switch (lineType) {
    case IMAGE:
      return image->get_height() + ITEMS_SPACE;
    case NORMAL_LEFT:
      return font->get_height() + ITEMS_SPACE;
    default:
      return font->get_height() + ITEMS_SPACE;
  }
}
