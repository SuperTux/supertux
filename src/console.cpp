//  $Id$
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
#include "scripting/squirrel_error.hpp"
#include "scripting/squirrel_util.hpp"
#include "physfs/physfs_stream.hpp"
#include "player_status.hpp"
#include "main.hpp"
#include "log.hpp"
#include "resources.hpp"

/// speed (pixels/s) the console closes
static const float FADE_SPEED = 1;

Console::Console()
  : history_position(history.end()), vm(NULL), backgroundOffset(0), 
    height(0), alpha(1.0), offset(0), focused(false), stayOpen(0) {
  fontheight = 8;
}

Console::~Console() 
{
  if(vm != NULL) {
    sq_release(Scripting::global_vm, &vm_object);
  }
}

void
Console::init_graphics()
{
  font.reset(new Font("images/engine/fonts/white-small.png",
                      "images/engine/fonts/shadow-small.png", 8, 9, 1));
  fontheight = font->get_height();
  background.reset(new Surface("images/engine/console.png"));
  background2.reset(new Surface("images/engine/console2.png"));
}

void 
Console::flush(ConsoleStreamBuffer* buffer) 
{
  if (buffer == &outputBuffer) {
    std::string s = outputBuffer.str();
    if ((s.length() > 0) && ((s[s.length()-1] == '\n') || (s[s.length()-1] == '\r'))) {
      while ((s[s.length()-1] == '\n') || (s[s.length()-1] == '\r')) s.erase(s.length()-1);
      addLines(s);
      outputBuffer.str(std::string());
    }
  }
  if (buffer == &inputBuffer) {
    std::string s = inputBuffer.str();
    if ((s.length() > 0) && ((s[s.length()-1] == '\n') || (s[s.length()-1] == '\r'))) {
      while ((s[s.length()-1] == '\n') || (s[s.length()-1] == '\r')) s.erase(s.length()-1);
      addLines("> "+s);
      parse(s);
      inputBuffer.str(std::string());
    }
  }
}

void
Console::ready_vm()
{
  if(vm == NULL) {
    vm = Scripting::global_vm;
    HSQUIRRELVM new_vm = sq_newthread(vm, 16);
    if(new_vm == NULL)
      throw Scripting::SquirrelError(vm, "Couldn't create new VM thread for console");

    // store reference to thread
    sq_resetobject(&vm_object);
    if(SQ_FAILED(sq_getstackobj(vm, -1, &vm_object)))
      throw Scripting::SquirrelError(vm, "Couldn't get vm object for console");
    sq_addref(vm, &vm_object);
    sq_pop(vm, 1);
    
    // create new roottable for thread
    sq_newtable(new_vm);
    sq_pushroottable(new_vm);
    if(SQ_FAILED(sq_setdelegate(new_vm, -2)))
      throw Scripting::SquirrelError(new_vm, "Couldn't set console_table delegate");

    sq_setroottable(new_vm);

    vm = new_vm;
    
    try {
      std::string filename = "scripts/console.nut";
      IFileStream stream(filename);
      Scripting::compile_and_run(vm, stream, filename);
    } catch(std::exception& e) {
      log_warning << "Couldn't load console.nut: " << e.what() << std::endl;
    }
  }
}

void
Console::execute_script(const std::string& command)
{
  using namespace Scripting;

  ready_vm(); 

  SQInteger oldtop = sq_gettop(vm); 
  try {
    if(SQ_FAILED(sq_compilebuffer(vm, command.c_str(), command.length(),
                 "", SQTrue)))
      throw SquirrelError(vm, "Couldn't compile command");

    sq_pushroottable(vm); 
    if(SQ_FAILED(sq_call(vm, 1, SQTrue, SQTrue)))
      throw SquirrelError(vm, "Problem while executing command");

    if(sq_gettype(vm, -1) != OT_NULL)
      addLines(squirrel2string(vm, -1));
  } catch(std::exception& e) {
    addLines(e.what());
  }
  SQInteger newtop = sq_gettop(vm);
  if(newtop < oldtop) {
    log_fatal << "Script destroyed squirrel stack..." << std::endl;
  } else {
    sq_settop(vm, oldtop);
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
Console::show_history(int offset)
{
  while ((offset > 0) && (history_position != history.end())) {
    history_position++;
    offset--;
  }
  while ((offset < 0) && (history_position != history.begin())) {
    history_position--;
    offset++;
  }
  if (history_position == history.end()) {
    inputBuffer.str(std::string());
  } else {
    inputBuffer.str(*history_position);
    inputBuffer.pubseekoff(0, std::ios_base::end, std::ios_base::out);
  }
}

// Helper functions for Console::autocomplete
// TODO: Fix rough documentation
namespace {

void sq_insert_commands(std::list<std::string>& cmds, HSQUIRRELVM vm, std::string table_prefix, std::string search_prefix);

/**
 * Acts upon key,value on top of stack:
 * Appends key (plus type-dependent suffix) to cmds if table_prefix+key starts with search_prefix;
 * Calls sq_insert_commands if search_prefix starts with table_prefix+key (and value is a table/class/instance);
 */
void
sq_insert_command(std::list<std::string>& cmds, HSQUIRRELVM vm, std::string table_prefix, std::string search_prefix)
{
  const SQChar* key_chars;
  if (SQ_FAILED(sq_getstring(vm, -2, &key_chars))) return;
  std::string key_string = table_prefix + key_chars;

  switch (sq_gettype(vm, -1)) {
    case OT_INSTANCE:
      key_string+=".";
      if (search_prefix.substr(0, key_string.length()) == key_string) {
        sq_getclass(vm, -1);
	sq_insert_commands(cmds, vm, key_string, search_prefix);
        sq_pop(vm, 1);
      }
      break;
    case OT_TABLE:
    case OT_CLASS:
      key_string+=".";
      if (search_prefix.substr(0, key_string.length()) == key_string) {
	sq_insert_commands(cmds, vm, key_string, search_prefix);
      }
      break;
    case OT_CLOSURE:
    case OT_NATIVECLOSURE:
      key_string+="()";
      break;
    default:
      break;
  }

  if (key_string.substr(0, search_prefix.length()) == search_prefix) {
    cmds.push_back(key_string);
  }

}

/**
 * calls sq_insert_command for all entries of table/class on top of stack
 */
void
sq_insert_commands(std::list<std::string>& cmds, HSQUIRRELVM vm, std::string table_prefix, std::string search_prefix)
{
  sq_pushnull(vm); // push iterator
  while (SQ_SUCCEEDED(sq_next(vm,-2))) {
    sq_insert_command(cmds, vm, table_prefix, search_prefix);
    sq_pop(vm, 2); // pop key, val
  }
  sq_pop(vm, 1); // pop iterator
}


} 
// End of Console::autocomplete helper functions

void
Console::autocomplete()
{
  std::string prefix = inputBuffer.str();
  addLines("> "+prefix);

  std::list<std::string> cmds;

  // append all known CCRs to list
  for (std::map<std::string, std::list<ConsoleCommandReceiver*> >::iterator i = commands.begin(); i != commands.end(); i++) {
    std::string cmdKnown = i->first;
    if (cmdKnown.substr(0, prefix.length()) == prefix) {
      cmds.push_back(cmdKnown);
    }
  }

  ready_vm();

  // append all keys of the current root table to list
  sq_pushroottable(vm); // push root table
  while(true) {
    // check all keys (and their children) for matches
    sq_insert_commands(cmds, vm, "", prefix);

    // cycle through parent(delegate) table
    SQInteger oldtop = sq_gettop(vm);
    if(SQ_FAILED(sq_getdelegate(vm, -1)) || oldtop == sq_gettop(vm)) {
      break;
    }
    sq_remove(vm, -2); // remove old table
  }
  sq_pop(vm, 1); // remove table

  // depending on number of hits, show matches or autocomplete
  if (cmds.size() == 0) addLines("No known command starts with \""+prefix+"\"");
  if (cmds.size() == 1) {
    inputBuffer.str(cmds.front());
    inputBuffer.pubseekoff(0, std::ios_base::end, std::ios_base::out);
  }
  if (cmds.size() > 1) {
    while (cmds.begin() != cmds.end()) {
      addLines(cmds.front());
      cmds.pop_front();
    }
  }
}

void 
Console::addLines(std::string s) 
{
  std::istringstream iss(s);
  std::string line;
  while (std::getline(iss, line, '\n')) addLine(line);
}

void 
Console::addLine(std::string s) 
{
  // output line to stderr
  std::cerr << s << std::endl;

  // wrap long lines
  std::string overflow;
  do {
    lines.push_front(Font::wrap_to_chars(s, 99, &overflow));
    s = overflow;
  } while (s.length() > 0);

  // trim scrollback buffer
  while (lines.size() >= 1000)
    lines.pop_back();
 
  // increase console height if necessary
  if (height < 64) {
    if(height < 4)
      height = 4;
    height += fontheight;
  }

  // reset console to full opacity
  alpha = 1.0;

  // increase time that console stays open
  if(stayOpen < 6)
    stayOpen += 1.5;
}

void
Console::parse(std::string s) 
{
  // make sure we actually have something to parse
  if (s.length() == 0) return;

  // add line to history
  history.push_back(s);
  history_position = history.end();

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
  if (args.size() == 0) return;
  std::string command = args.front();
  args.erase(args.begin());

  // ignore if it's an internal command
  if (consoleCommand(command,args)) return;

  // look up registered ccr
  std::map<std::string, std::list<ConsoleCommandReceiver*> >::iterator i = commands.find(command);
  if ((i == commands.end()) || (i->second.size() == 0)) {
    try {
      execute_script(s);
    } catch(std::exception& e) {
      addLines(e.what());
    }
    return;
  }

  // send command to the most recently registered ccr
  ConsoleCommandReceiver* ccr = i->second.front();
  if (ccr->consoleCommand(command, args) != true) log_warning << "Sent command to registered ccr, but command was unhandled" << std::endl;
}

bool
Console::consoleCommand(std::string command, std::vector<std::string> arguments) 
{
  if (command == "ccrs") {
    if (arguments.size() != 1) {
      log_info << "Usage: ccrs <command>" << std::endl;
      return true;
    }
    std::map<std::string, std::list<ConsoleCommandReceiver*> >::iterator i = commands.find(arguments[0]);
    if ((i == commands.end()) || (i->second.size() == 0)) {
      log_info << "unknown command: \"" << arguments[0] << "\"" << std::endl;
      return true;
    }

    std::ostringstream ccr_list;
    std::list<ConsoleCommandReceiver*> &ccrs = i->second;
    std::list<ConsoleCommandReceiver*>::iterator j;
    for (j = ccrs.begin(); j != ccrs.end(); j++) {
      if (j != ccrs.begin()) ccr_list << ", ";
      ccr_list << "[" << *j << "]";
    }

    log_info << "registered ccrs for \"" << arguments[0] << "\": " << ccr_list.str() << std::endl;
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
  alpha = 1.0;
  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
}

void 
Console::hide()
{
  focused = false;
  height = 0;
  stayOpen = 0;

  // clear input buffer
  inputBuffer.str(std::string());
  SDL_EnableKeyRepeat(0, SDL_DEFAULT_REPEAT_INTERVAL);
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
Console::update(float elapsed_time)
{
  if(stayOpen > 0) {
    stayOpen -= elapsed_time;
    if(stayOpen < 0)
      stayOpen = 0;
  } else if(!focused && height > 0) {
    alpha -= elapsed_time * FADE_SPEED;
    if(alpha < 0) {
      alpha = 0;
      height = 0;
    }
  }
}

void 
Console::draw(DrawingContext& context)
{
  if (height == 0)
    return;

  int layer = LAYER_GUI + 1;

  context.push_transform();
  context.set_alpha(alpha);
  context.draw_surface(background2.get(), Vector(SCREEN_WIDTH/2 - background->get_width()/2 - background->get_width() + backgroundOffset, height - background->get_height()), layer);
  context.draw_surface(background2.get(), Vector(SCREEN_WIDTH/2 - background->get_width()/2 + backgroundOffset, height - background->get_height()), layer);
  context.draw_surface(background.get(), Vector(SCREEN_WIDTH/2 - background->get_width()/2, height - background->get_height()), layer);
  backgroundOffset+=10;
  if (backgroundOffset > (int)background->get_width()) backgroundOffset -= (int)background->get_width();

  int lineNo = 0;

  if (focused) {
    lineNo++;
    float py = height-4-1*9;
    context.draw_text(font.get(), "> "+inputBuffer.str()+"_", Vector(4, py), LEFT_ALLIGN, layer);
  }

  int skipLines = -offset;
  for (std::list<std::string>::iterator i = lines.begin(); i != lines.end(); i++) {
    if (skipLines-- > 0) continue;
    lineNo++;
    float py = height-4-lineNo*9;
    if (py < -9) break;
    context.draw_text(font.get(), *i, Vector(4, py), LEFT_ALLIGN, layer);
  }

  context.pop_transform();
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
    log_warning << "Command \"" << command << "\" not associated with a command receiver. Not dissociated." << std::endl;
    return;
  }
  std::list<ConsoleCommandReceiver*>::iterator j = find(i->second.begin(), i->second.end(), ccr);
  if (j == i->second.end()) {
    log_warning << "Command \"" << command << "\" not associated with given command receiver. Not dissociated." << std::endl;
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

Console* Console::instance = NULL;
ConsoleStreamBuffer Console::inputBuffer;
ConsoleStreamBuffer Console::outputBuffer;
std::ostream Console::input(&Console::inputBuffer);
std::ostream Console::output(&Console::outputBuffer);

