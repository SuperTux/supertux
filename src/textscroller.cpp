//  $Id$
// 
//  SuperTux
//  Copyright (C) 2005 Matthias Braun <matze@braunis.de>
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.
#include <config.h>

#include "textscroller.hpp"

#include <stdexcept>
#include "log.hpp"
#include "mainloop.hpp"
#include "resources.hpp"
#include "video/font.hpp"
#include "video/drawing_context.hpp"
#include "video/surface.hpp"
#include "video/screen.hpp"
#include "gui/menu.hpp"
#include "lisp/parser.hpp"
#include "lisp/lisp.hpp"
#include "audio/sound_manager.hpp"
#include "main.hpp"
#include "control/joystickkeyboardcontroller.hpp"

static const float DEFAULT_SPEED = 20;
static const float LEFT_BORDER = 50;
static const float SCROLL = 60;
static const float ITEMS_SPACE = 4;

static void split_text(const std::string& text, std::vector<std::string>& lines)
{
  // Split text string lines into a vector
  lines.clear();
  std::string::size_type i, l;
  i = 0;
  while(true) {
    l = text.find("\n", i);

    if(l == std::string::npos) {
      lines.push_back(text.substr(i, text.size()-i));
      break;
    }

    lines.push_back(text.substr(i, l-i));
    i = l+1;
  }
}

TextScroller::TextScroller(const std::string& filename)
{
  defaultspeed = DEFAULT_SPEED;
  speed = defaultspeed;
  
  std::string text;
  std::string background_file;

  lisp::Parser parser;
  try {
    std::auto_ptr<lisp::Lisp> root (parser.parse(filename));

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
  split_text(text, lines);

  for(size_t i = 0; i < lines.size(); ++i) {
    const std::string& line = lines[i];
    if(line.size() == 0)
      continue;
    if(line[0] == '!') {
      std::string imagename = line.substr(1, line.size()-1);
      images.insert(std::make_pair(imagename, new Surface(imagename)));
    }
  }

  // load background image
  background.reset(new Surface("images/background/" + background_file));

  scroll = 0;
}

TextScroller::~TextScroller()
{
  for(std::map<std::string, Surface*>::iterator i = images.begin();
      i != images.end(); ++i)
    delete i->second; 
}

void
TextScroller::setup()
{
  sound_manager->play_music(music);
  Menu::set_current(NULL);
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
    fadeout(500);
    main_loop->exit_screen();
  }

  scroll += speed * elapsed_time;
    
  if(scroll < 0)
    scroll = 0;
}

void
TextScroller::draw(DrawingContext& context)
{
  context.draw_surface(background.get(), Vector(0,0), 0);

  float y = SCREEN_HEIGHT - scroll;
  for(size_t i = 0; i < lines.size(); i++) {
    const std::string& line = lines[i];
    if(line.size() == 0) {
      y += white_text->get_height() + ITEMS_SPACE;
      continue;
    }
      
    const Font* font = 0;
    const Surface* image = 0;
    bool center = true;
    switch(line[0])
    {
      case ' ': font = white_small_text; break;
      case '\t': font = white_text; break;
      case '-': font = white_big_text; break;
      case '*': font = blue_text; break;
      case '#': font = white_text; center = false; break;
      case '!': {
                  std::string imagename = line.substr(1, line.size()-1);
                  image = images[imagename];
                  break;
                }
      default:
                log_warning << "text contains an unformated line" << std::endl;
                font = white_text;
                center = false;
                break;
    }
    
    if(font != 0) {
      if(center) {
        context.draw_text(font,
            line.substr(1, line.size()-1),
            Vector(SCREEN_WIDTH/2, y),
            CENTER_ALLIGN, LAYER_FOREGROUND1);
      } else {
        context.draw_text(font,
            line.substr(1, line.size()-1),
            Vector(LEFT_BORDER, y),
            LEFT_ALLIGN, LAYER_FOREGROUND1);
      }
      y += font->get_height() + ITEMS_SPACE;
    }
    if(image != 0) {
      context.draw_surface(image,
          Vector( (SCREEN_WIDTH - image->get_width()) / 2, y), 255);
      y += image->get_height() + ITEMS_SPACE;
    }
  }

  if(y < 0) {
    fadeout(500); 
    main_loop->exit_screen();
  }
}

InfoBox::InfoBox(const std::string& text)
  : firstline(0)
{
  split_text(text, lines);

  for(size_t i = 0; i < lines.size(); ++i) {
    if(lines[i].size() == 0)
      continue;
    if(lines[i][0] == '!') {
      std::string imagename = lines[i].substr(1, lines[i].size()-1);
      images.insert(std::make_pair(imagename, new Surface(imagename)));
    }
  }

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
  for(std::map<std::string, Surface*>::iterator i = images.begin();
    i != images.end(); ++i)
    delete i->second;
  delete arrow_scrollup;
  delete arrow_scrolldown;
}

void
InfoBox::draw(DrawingContext& context)
{
  const Font* heading_font = white_big_text;
  const Font* normal_font = white_text;
  const Font* small_font = white_small_text;
  const Font* reference_font = blue_text;

  float x1 = 200;
  float y1 = 100;
  float width = 400;
  float height = 200;

  context.draw_filled_rect(Vector(x1, y1), Vector(width, height),
      Color(0.6f, 0.7f, 0.8f, 0.5f), LAYER_GUI-1);

  float y = y1;
  for(size_t i = firstline; i < lines.size(); ++i) {
    const std::string& line = lines[i];
    if(y >= y1 + height)
      break;

    if(line.size() == 0) {
      y += normal_font->get_height() + ITEMS_SPACE;    
      continue;                                        
    }

    const Font* font = 0;
    const Surface* image = 0;
    bool center = true;
    switch(line[0])
    {
      case ' ': font = small_font; break;
      case '\t': font = normal_font; break;
      case '-': font = heading_font; break;
      case '*': font = reference_font; break;
      case '#': font = normal_font; center = false; break;
      case '!': {
        std::string imagename = line.substr(1, line.size()-1);
        image = images[imagename];
        break;
      }
      default:
        log_warning << "text contains an unformatted line" << std::endl;
        font = normal_font;
        center = false;
        break;
    }

    if(image != 0) {
      context.draw_surface(image,
      Vector( (SCREEN_WIDTH - image->get_width()) / 2,
              y), LAYER_GUI);
      y += image->get_height() + ITEMS_SPACE;
    } else if(center) {
      context.draw_text(font,
          line.substr(1, line.size()-1),
          Vector(SCREEN_WIDTH/2, y),
          CENTER_ALLIGN, LAYER_GUI);
      y += font->get_height() + ITEMS_SPACE;
    } else {
      context.draw_text(font,
          line.substr(1, line.size()-1),
          Vector(x1, y),
          LEFT_ALLIGN, LAYER_GUI);
      y += font->get_height() + ITEMS_SPACE;
    }

    // draw the scrolling arrows
    if (arrow_scrollup && firstline > 0)
      context.draw_surface(arrow_scrollup,
      Vector( x1 + width  - arrow_scrollup->get_width(),  // top-right corner of box
              y1), LAYER_GUI);

    if (arrow_scrolldown && firstline < lines.size()-1)
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

