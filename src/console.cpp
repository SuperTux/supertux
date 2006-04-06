//  $Id: worldmap.cpp 3209 2006-04-02 22:19:22Z sommer $
//
//  SuperTux - Console
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include <config.h>
#include <iostream>
#include "console.hpp"
#include "video/drawing_context.hpp"
#include "video/surface.hpp"
#include "player_status.hpp"
#include "main.hpp"
#include "resources.hpp"

namespace {
  int ticks; // TODO: use a clock?
}

Console::Console(DrawingContext* context) : context(context) 
{
  background = new Surface("images/engine/console.jpg");
}

Console::~Console() 
{
  delete background;
}

void 
Console::flush() 
{
 lines.push_front(outputBuffer.str());
 if (lines.size() >= 256) lines.pop_back();
 if (height < 64) {
   if (height < 4) height=4;
   height+=9;
 }
 ticks=120;
 std::cerr << outputBuffer.str() << std::flush;
 outputBuffer.str(std::string());
}

void 
Console::draw() 
{
  if (height == 0) return;
  if (ticks-- < 0) {
    height-=1;
    ticks=0;
    if (height < 0) height=0;
  }
  if (height == 0) return;

  context->draw_surface(background, Vector(SCREEN_WIDTH/2 - background->get_width()/2, height - background->get_height()), LAYER_FOREGROUND1+1);

  int lineNo = 0;
  for (std::list<std::string>::iterator i = lines.begin(); i != lines.end(); i++) {
    lineNo++;
    float py = height-4-lineNo*9;
    if (py < -9) break;
    context->draw_text(white_small_text, *i, Vector(BORDER_X, py), LEFT_ALLIGN, LAYER_FOREGROUND1+1);
  }
}

int Console::height = 0;
std::list<std::string> Console::lines;
ConsoleStreamBuffer Console::outputBuffer;
std::ostream Console::output(&Console::outputBuffer);

