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
  background = new Surface("images/engine/console.png");
  background2 = new Surface("images/engine/console2.png");
}

Console::~Console() 
{
  delete background;
  delete background2;
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
  std::cerr << s << std::endl;
  while (s.length() > 99) {
    lines.push_front(s.substr(0, 99-3)+"...");
    s = "..."+s.substr(99-3);
  }
  lines.push_front(s);
  while (lines.size() >= 65535) lines.pop_back();
  if (height < 64) {
    if (height < 4+9) height=4+9;
    height+=9;
  }
  ticks=60;
}

void
Console::parse(std::string s) 
{
  // make sure we actually have something to parse
  if (s.length() == 0) return;
	
  // split line into list of args
  std::vector<std::string> args;
  size_t start = 0;
  size_t end = 0;
  while (1) {
    start = s.find_first_not_of(" ,", end);
    end = s.find_first_of(" ,", start);
    if (start == s.npos) break;
    args.push_back(s.substr(start, end-start));
  }

  // command is args[0]
  std::string command = args.front();
  args.erase(args.begin());

  // ignore if it's an internal command
  if (consoleCommand(command,args)) return;

  // look up registered ccr
  std::map<std::string, std::list<ConsoleCommandReceiver*> >::iterator i = commands.find(command);
  if ((i == commands.end()) || (i->second.size() == 0)) {
    addLine("unknown command: \"" + command + "\"");
    return;
  }

  // send command to the most recently registered ccr
  ConsoleCommandReceiver* ccr = i->second.front();
  if (ccr->consoleCommand(command, args) != true) msg_warning << "Sent command to registered ccr, but command was unhandled" << std::endl;
}

bool
Console::consoleCommand(std::string command, std::vector<std::string> arguments) 
{
  if (command == "ccrs") {
    if (arguments.size() != 1) {
      msg_info << "Usage: ccrs <command>" << std::endl;
      return true;
    }
    std::map<std::string, std::list<ConsoleCommandReceiver*> >::iterator i = commands.find(arguments[0]);
    if ((i == commands.end()) || (i->second.size() == 0)) {
      msg_info << "unknown command: \"" << arguments[0] << "\"" << std::endl;
      return true;
    }

    std::ostringstream ccr_list;
    std::list<ConsoleCommandReceiver*> &ccrs = i->second;
    std::list<ConsoleCommandReceiver*>::iterator j;
    for (j = ccrs.begin(); j != ccrs.end(); j++) {
      if (j != ccrs.begin()) ccr_list << ", ";
      ccr_list << "[" << *j << "]";
    }

    msg_info << "registered ccrs for \"" << arguments[0] << "\": " << ccr_list.str() << std::endl;
    return true;
  }

  return false;
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

  context.draw_surface(background2, Vector(SCREEN_WIDTH/2 - background->get_width()/2 - background->get_width() + backgroundOffset, height - background->get_height()), LAYER_FOREGROUND1+1);
  context.draw_surface(background2, Vector(SCREEN_WIDTH/2 - background->get_width()/2 + backgroundOffset, height - background->get_height()), LAYER_FOREGROUND1+1);
  context.draw_surface(background, Vector(SCREEN_WIDTH/2 - background->get_width()/2, height - background->get_height()), LAYER_FOREGROUND1+1);
  backgroundOffset+=10;
  if (backgroundOffset > (int)background->get_width()) backgroundOffset -= (int)background->get_width();

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
    msg_warning << "Command \"" << command << "\" not associated with a command receiver. Not dissociated." << std::endl;
    return;
  }
  std::list<ConsoleCommandReceiver*>::iterator j = find(i->second.begin(), i->second.end(), ccr);
  if (j == i->second.end()) {
    msg_warning << "Command \"" << command << "\" not associated with given command receiver. Not dissociated." << std::endl;
    return;
  }
  i->second.erase(j);
}

void 
Console::unregisterCommands(ConsoleCommandReceiver* ccr)
{
  for (std::map<std::string, std::list<ConsoleCommandReceiver*> >::iterator i = commands.begin(); i != commands.end(); i++) {
    std::list<ConsoleCommandReceiver*> &ccrs = i->second;
    std::list<ConsoleCommandReceiver*>::iterator j;
    while ((j = find(ccrs.begin(), ccrs.end(), ccr)) != ccrs.end()) {
      ccrs.erase(j);
    }
  }
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
int Console::backgroundOffset = 0;

