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

#include "textscroller.h"

#include <stdexcept>
#include "resources.h"
#include "video/font.h"
#include "video/drawing_context.h"
#include "lisp/parser.h"
#include "lisp/lisp.h"
#include "main.h"
#include "control/joystickkeyboardcontroller.h"

static const float DEFAULT_SPEED = .02;
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

void display_text_file(const std::string& filename)
{
  const Font* heading_font = white_big_text;
  const Font* normal_font = white_text;
  const Font* small_font = white_small_text;
  const Font* reference_font = blue_text;
  float defaultspeed = DEFAULT_SPEED;
  float speed = defaultspeed;
  
  std::string text;
  std::string background_file;
  std::vector<std::string> lines;

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
    if(text_lisp->get("speed", defaultspeed))
      defaultspeed /= 50;
  } catch(std::exception& e) {
    std::cerr << "Couldn't load file '" << filename << "': " << e.what() <<
      "\n";
    return;
  }

  // Split text string lines into a vector
  split_text(text, lines);

  // load background image
  Surface* background 
    = new Surface("images/background/" + background_file, false);

  bool done = false;
  float scroll = 0;
  float left_border = 50;

  DrawingContext context;
  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

  Uint32 lastticks = SDL_GetTicks();
  while(!done) {
    main_controller->update();
    /* in case of input, exit */
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
      main_controller->process_event(event);
      if(event.type == SDL_QUIT)
        throw std::runtime_error("received window close");
    }

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
    if(main_controller->pressed(Controller::PAUSE_MENU))
      done = true;
    
    /* draw the credits */
    context.draw_surface(background, Vector(0,0), 0);

    float y = 0;
    for(size_t i = 0; i < lines.size(); i++) {
      const std::string& line = lines[i];
      if(line.size() == 0) {
        y += normal_font->get_height() + ITEMS_SPACE;
        continue;
      }
      
      const Font* font = 0;
      bool center = true;
      switch(line[0])
      {
        case ' ': font = small_font; break;
        case '\t': font = normal_font; break;
        case '-': font = heading_font; break;
        case '*': font = reference_font; break;
        case '#': font = normal_font; center = false; break;
        default:
          std::cerr << "Warning: text contains an unformated line.\n";
          font = normal_font;
          center = false;
          break;
      }
      
      if(center) {
        context.draw_text(font,
                          line.substr(1, line.size()-1),
                          Vector(SCREEN_WIDTH/2, SCREEN_HEIGHT + y - scroll),
                          CENTER_ALLIGN, LAYER_FOREGROUND1);
      } else {
        context.draw_text(font,
                          line.substr(1, line.size()-1),
                          Vector(left_border, SCREEN_HEIGHT + y - scroll),
                          LEFT_ALLIGN, LAYER_FOREGROUND1);
      }
      
      y += font->get_height() + ITEMS_SPACE;
    }
    
    context.do_drawing();
    
    if(SCREEN_HEIGHT+y-scroll < 0 && 20+SCREEN_HEIGHT+y-scroll < 0)
      done = 1;
    
    Uint32 ticks = SDL_GetTicks();
    scroll += speed * (ticks - lastticks);
    lastticks = ticks;
    if(scroll < 0)
      scroll = 0;
    
    SDL_Delay(10);
  }
  
  SDL_EnableKeyRepeat(0, 0);    // disables key repeating
  delete background;
}

InfoBox::InfoBox(const std::string& text)
  : firstline(0)
{
  split_text(text, lines);
}

InfoBox::~InfoBox()
{
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
      Color(150, 180, 200, 125), LAYER_GUI-1);

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
    bool center = true;
    switch(line[0])
    {
      case ' ': font = small_font; break;
      case '\t': font = normal_font; break;
      case '-': font = heading_font; break;
      case '*': font = reference_font; break;
      case '#': font = normal_font; center = false; break;
      default:
        std::cerr << "Warning: text contains an unformated line.\n";
        font = normal_font;
        center = false;
        break;
    }
    
    if(center) {
      context.draw_text(font,
          line.substr(1, line.size()-1),
          Vector(SCREEN_WIDTH/2, y),
          CENTER_ALLIGN, LAYER_GUI);
    } else {
      context.draw_text(font,
          line.substr(1, line.size()-1),
          Vector(x1, y),
          LEFT_ALLIGN, LAYER_GUI);
    }
      
    y += font->get_height() + ITEMS_SPACE;
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

