//  SuperTux - Console
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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
//#include <config.h>

#include "supertux/console.hpp"

//#include <SDL_keyboard.h>
//#include <SDL_timer.h>
//#include <iostream>
#include <math.h>

#include "physfs/physfs_stream.hpp"
//#include "scripting/squirrel_error.hpp"
#include "scripting/squirrel_util.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/main.hpp"
//#include "supertux/player_status.hpp"
//#include "supertux/resources.hpp"
//#include "util/log.hpp"
#include "video/drawing_context.hpp"
//#include "video/surface.hpp"

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
  font.reset(new Font(Font::FIXED,"fonts/andale12.stf",1));
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
Console::input(char c)
{
  inputBuffer.insert(inputBufferPosition, 1, c);
  inputBufferPosition++;
}

void
Console::backspace()
{
  if ((inputBufferPosition > 0) && (inputBuffer.length() > 0)) {
    inputBuffer.erase(inputBufferPosition-1, 1);
    inputBufferPosition--;
  }
}

void
Console::eraseChar()
{
  if (inputBufferPosition < (int)inputBuffer.length()) {
    inputBuffer.erase(inputBufferPosition, 1);
  }
}

void
Console::enter()
{
  addLines("> "+inputBuffer);
  parse(inputBuffer);
  inputBuffer = "";
  inputBufferPosition = 0;
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
    inputBuffer = "";
    inputBufferPosition = 0;
  } else {
    inputBuffer = *history_position;
    inputBufferPosition = inputBuffer.length();
  }
}

void 
Console::move_cursor(int offset)
{
  if (offset == -65535) inputBufferPosition = 0;
  if (offset == +65535) inputBufferPosition = inputBuffer.length();
  inputBufferPosition+=offset;
  if (inputBufferPosition < 0) inputBufferPosition = 0;
  if (inputBufferPosition > (int)inputBuffer.length()) inputBufferPosition = inputBuffer.length();
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
  //int autocompleteFrom = inputBuffer.find_last_of(" ();+", inputBufferPosition);
  int autocompleteFrom = inputBuffer.find_last_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_->.", inputBufferPosition);
  if (autocompleteFrom != (int)std::string::npos) {
    autocompleteFrom += 1;
  } else {
    autocompleteFrom = 0;
  }
  std::string prefix = inputBuffer.substr(autocompleteFrom, inputBufferPosition - autocompleteFrom);
  addLines("> "+prefix);

  std::list<std::string> cmds;

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
    // one match: just replace input buffer with full command
    std::string replaceWith = cmds.front();
    inputBuffer.replace(autocompleteFrom, prefix.length(), replaceWith);
    inputBufferPosition += (replaceWith.length() - prefix.length());
  }
  if (cmds.size() > 1) {
    // multiple matches: show all matches and set input buffer to longest common prefix
    std::string commonPrefix = cmds.front();
    while (cmds.begin() != cmds.end()) {
      std::string cmd = cmds.front();
      cmds.pop_front();
      addLines(cmd);
      for (int n = commonPrefix.length(); n >= 1; n--) {
        if (cmd.compare(0, n, commonPrefix) != 0) commonPrefix.resize(n-1); else break;
      }
    }
    std::string replaceWith = commonPrefix;
    inputBuffer.replace(autocompleteFrom, prefix.length(), replaceWith);
    inputBufferPosition += (replaceWith.length() - prefix.length());
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
  unsigned int line_count = 0;
  do {
    lines.push_front(Font::wrap_to_chars(s, 99, &overflow));
    line_count++;
    s = overflow;
  } while (s.length() > 0);

  // trim scrollback buffer
  while (lines.size() >= 1000)
    lines.pop_back();

  // increase console height if necessary
  if (height < 64) {
    if(height < 4)
      height = 4;
    height += fontheight * line_count;
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

  try {
    execute_script(s);
  } catch(std::exception& e) {
    addLines(e.what());
  }

}

bool
Console::consoleCommand(std::string /*command*/, std::vector<std::string> /*arguments*/)
{
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
  if(!config->console_enabled)
    return;

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
  inputBuffer = "";
  inputBufferPosition = 0;
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
  for (int x = (SCREEN_WIDTH/2 - background->get_width()/2 - (static_cast<int>(ceilf((float)SCREEN_WIDTH / (float)background->get_width()) - 1) * background->get_width())); x < SCREEN_WIDTH; x+=background->get_width()) {
    context.draw_surface(background.get(), Vector(x, height - background->get_height()), layer);
  }
  backgroundOffset+=10;
  if (backgroundOffset > (int)background->get_width()) backgroundOffset -= (int)background->get_width();

  int lineNo = 0;

  if (focused) {
    lineNo++;
    float py = height-4-1 * font->get_height();
    context.draw_text(font.get(), "> "+inputBuffer, Vector(4, py), ALIGN_LEFT, layer);
    if (SDL_GetTicks() % 1000 < 750) {
      int cursor_px = 2 + inputBufferPosition;
      context.draw_text(font.get(), "_", Vector(4 + (cursor_px * font->get_text_width("X")), py), ALIGN_LEFT, layer);
    }
  }

  int skipLines = -offset;
  for (std::list<std::string>::iterator i = lines.begin(); i != lines.end(); i++) {
    if (skipLines-- > 0) continue;
    lineNo++;
    float py = height - 4 - lineNo*font->get_height();
    if (py < -font->get_height()) break;
    context.draw_text(font.get(), *i, Vector(4, py), ALIGN_LEFT, layer);
  }
  context.pop_transform();
}

Console* Console::instance = NULL;
int Console::inputBufferPosition = 0;
std::string Console::inputBuffer;
ConsoleStreamBuffer Console::outputBuffer;
std::ostream Console::output(&Console::outputBuffer);

