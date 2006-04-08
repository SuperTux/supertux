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

Console::Console()
{
  background = new Surface("images/engine/console.jpg");
}

Console::~Console() 
{
  delete background;
}

void 
Console::flush(ConsoleStreamBuffer* buffer) 
{
  if (buffer == &outputBuffer) {
    std::string s = outputBuffer.str();
    if ((s.length() > 0) && ((s[s.length()-1] == '\n') || (s[s.length()-1] == '\r'))) {
      while ((s[s.length()-1] == '\n') || (s[s.length()-1] == '\r')) s.erase(s.length()-1);
      addLine(s);
      outputBuffer.str(std::string());
    }
  }
  if (buffer == &inputBuffer) {
    std::string s = inputBuffer.str();
    if ((s.length() > 0) && ((s[s.length()-1] == '\n') || (s[s.length()-1] == '\r'))) {
      while ((s[s.length()-1] == '\n') || (s[s.length()-1] == '\r')) s.erase(s.length()-1);
      addLine("> "+s);
      parse(s);
      inputBuffer.str(std::string());
    }
  }
}

void
Console::backspace()
{
  std::string s = inputBuffer.str();
  if (s.length() > 0) {
    s.erase(s.length()-1);
    inputBuffer.str(s);
    inputBuffer.pubseekoff(0, std::ios_base::end, std::ios_base::out);
  }
}

void
Console::scroll(int numLines)
{
  offset += numLines;
  if (offset > 0) offset = 0;
}

void
Console::autocomplete()
{
  std::string cmdPart = inputBuffer.str();
  addLine("> "+cmdPart);

  std::string cmdList = "";
  int cmdListLen = 0;
  for (std::map<std::string, std::list<ConsoleCommandReceiver*> >::iterator i = commands.begin(); i != commands.end(); i++) {
    std::string cmdKnown = i->first;
    if (cmdKnown.substr(0, cmdPart.length()) == cmdPart) {
      if (cmdListLen > 0) cmdList = cmdList + ", ";
      cmdList = cmdList + cmdKnown;
      cmdListLen++;
    }
  }
  if (cmdListLen == 0) addLine("No known command starts with \""+cmdPart+"\"");
  if (cmdListLen == 1) {
    inputBuffer.str(cmdList);
    inputBuffer.pubseekoff(0, std::ios_base::end, std::ios_base::out);
  }
  if (cmdListLen > 1) addLine(cmdList);
}

void 
Console::addLine(std::string s) 
{
  lines.push_front(s);
  if (lines.size() >= 65535) lines.pop_back();
  if (height < 64) {
    if (height < 4+9) height=4+9;
    height+=9;
  }
  ticks=60;
  std::cerr << s << std::endl;
}

void
Console::parse(std::string s) 
{
  std::map<std::string, std::list<ConsoleCommandReceiver*> >::iterator i = commands.find(s);
  if ((i == commands.end()) || (i->second.size() == 0)) {
    addLine("unknown command: \"" + s + "\"");
    return;
  }

  // send command to the most recently registered ccr
  ConsoleCommandReceiver* ccr = i->second.front();
  if (ccr->consoleCommand(s) != true) msg_warning("Sent command to registered ccr, but command was unhandled");
}

bool
Console::hasFocus() 
{
  return focused;
}

void
Console::show()
{
  focused = true;
  height = 256;
}

void 
Console::hide()
{
  focused = false;
  height = 0;

  // clear input buffer
  inputBuffer.str(std::string());
}

void 
Console::toggle()
{
  if (Console::hasFocus()) {
    Console::hide(); 
  } 
  else { 
    Console::show();
  }
}

void 
Console::draw(DrawingContext& context)
{
  if (height == 0) return;
  if (!focused) {
    if (ticks-- < 0) {
      height-=10;
      ticks=0;
      if (height < 0) height=0;
    }
    if (height == 0) return;
  }

  context.draw_surface(background, Vector(SCREEN_WIDTH/2 - background->get_width()/2, height - background->get_height()), LAYER_FOREGROUND1+1);

  int lineNo = 0;

  if (focused) {
    lineNo++;
    float py = height-4-1*9;
    context.draw_text(white_small_text, "> "+inputBuffer.str()+"_", Vector(4, py), LEFT_ALLIGN, LAYER_FOREGROUND1+1);
  }

  int skipLines = -offset;
  for (std::list<std::string>::iterator i = lines.begin(); i != lines.end(); i++) {
    if (skipLines-- > 0) continue;
    lineNo++;
    float py = height-4-lineNo*9;
    if (py < -9) break;
    context.draw_text(white_small_text, *i, Vector(4, py), LEFT_ALLIGN, LAYER_FOREGROUND1+1);
  }
}

void 
Console::registerCommand(std::string command, ConsoleCommandReceiver* ccr)
{
  commands[command].push_front(ccr);
}

void 
Console::unregisterCommand(std::string command, ConsoleCommandReceiver* ccr)
{
  std::map<std::string, std::list<ConsoleCommandReceiver*> >::iterator i = commands.find(command);
  if ((i == commands.end()) || (i->second.size() == 0)) {
    msg_warning("Command \"" << command << "\" not associated with a command receiver. Not dissociated.");
    return;
  }
  std::list<ConsoleCommandReceiver*>::iterator j = find(i->second.begin(), i->second.end(), ccr);
  if (j == i->second.end()) {
    msg_warning("Command \"" << command << "\" not associated with given command receiver. Not dissociated.");
    return;
  }
  i->second.erase(j);
}

int Console::height = 0;
bool Console::focused = false;
std::list<std::string> Console::lines;
std::map<std::string, std::list<ConsoleCommandReceiver*> > Console::commands;
ConsoleStreamBuffer Console::inputBuffer;
ConsoleStreamBuffer Console::outputBuffer;
std::ostream Console::input(&Console::inputBuffer);
std::ostream Console::output(&Console::outputBuffer);
int Console::offset = 0;

