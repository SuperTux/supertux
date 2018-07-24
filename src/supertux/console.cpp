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

#include "supertux/console.hpp"

#include "physfs/ifile_stream.hpp"
#include "scripting/scripting.hpp"
#include "scripting/squirrel_util.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "util/log.hpp"
#include "video/drawing_context.hpp"
#include "video/surface.hpp"

/// speed (pixels/s) the console closes
static const float FADE_SPEED = 1;

ConsoleBuffer::ConsoleBuffer() :
  m_lines(),
  m_console(nullptr)
{
}

void
ConsoleBuffer::set_console(Console* console)
{
  assert(!m_console);
  assert(console);

  m_console = console;
}

void
ConsoleBuffer::addLines(const std::string& s)
{
  std::istringstream iss(s);
  std::string line;
  while (std::getline(iss, line, '\n'))
  {
    addLine(line);
  }
}

void
ConsoleBuffer::addLine(const std::string& s_)
{
  std::string s = s_;

  // output line to stderr
  std::cerr << s << std::endl;

  // wrap long lines
  std::string overflow;
  int line_count = 0;
  do {
    m_lines.push_front(Font::wrap_to_chars(s, 99, &overflow));
    line_count += 1;
    s = overflow;
  } while (s.length() > 0);

  // trim scrollback buffer
  while (m_lines.size() >= 1000)
  {
    m_lines.pop_back();
  }

  if (m_console)
  {
    m_console->on_buffer_change(line_count);
  }
}

void
ConsoleBuffer::flush(ConsoleStreamBuffer& buffer)
{
  if (&buffer == &s_outputBuffer)
  {
    std::string s = s_outputBuffer.str();
    if ((s.length() > 0) && ((s[s.length()-1] == '\n') || (s[s.length()-1] == '\r')))
    {
      while ((s[s.length()-1] == '\n') || (s[s.length()-1] == '\r'))
      {
        s.erase(s.length()-1);
      }
      addLines(s);
      s_outputBuffer.str(std::string());
    }
  }
}

Console::Console(ConsoleBuffer& buffer) :
  m_buffer(buffer),
  m_inputBuffer(),
  m_inputBufferPosition(0),
  m_history(),
  m_history_position(m_history.end()),
  m_background(Surface::create("images/engine/console.png")),
  m_background2(Surface::create("images/engine/console2.png")),
  m_vm(NULL),
  m_vm_object(),
  m_backgroundOffset(0),
  m_height(0),
  m_alpha(1.0),
  m_offset(0),
  m_focused(false),
  m_font(new Font(Font::FIXED, "fonts/andale12.stf", 1)),
  m_stayOpen(0)
{
  buffer.set_console(this);
}

Console::~Console()
{
  if (m_vm != NULL && scripting::global_vm != NULL)
  {
    sq_release(scripting::global_vm, &m_vm_object);
  }
}

void
Console::on_buffer_change(int line_count)
{
  // increase console height if necessary
  if (m_stayOpen > 0 && m_height < 64)
  {
    if(m_height < 4)
    {
      m_height = 4;
    }
    m_height += m_font->get_height() * line_count;
  }

  // reset console to full opacity
  m_alpha = 1.0;
}

void
Console::ready_vm()
{
  if(m_vm == NULL) {
    m_vm = scripting::global_vm;
    HSQUIRRELVM new_vm = sq_newthread(m_vm, 16);
    if(new_vm == NULL)
      throw scripting::SquirrelError(m_vm, "Couldn't create new VM thread for console");

    // store reference to thread
    sq_resetobject(&m_vm_object);
    if(SQ_FAILED(sq_getstackobj(m_vm, -1, &m_vm_object)))
      throw scripting::SquirrelError(m_vm, "Couldn't get vm object for console");
    sq_addref(m_vm, &m_vm_object);
    sq_pop(m_vm, 1);

    // create new roottable for thread
    sq_newtable(new_vm);
    sq_pushroottable(new_vm);
    if(SQ_FAILED(sq_setdelegate(new_vm, -2)))
      throw scripting::SquirrelError(new_vm, "Couldn't set console_table delegate");

    sq_setroottable(new_vm);

    m_vm = new_vm;

    try {
      std::string filename = "scripts/console.nut";
      IFileStream stream(filename);
      scripting::compile_and_run(m_vm, stream, filename);
    } catch(std::exception& e) {
      log_warning << "Couldn't load console.nut: " << e.what() << std::endl;
    }
  }
}

void
Console::execute_script(const std::string& command)
{
  using namespace scripting;

  ready_vm();

  SQInteger oldtop = sq_gettop(m_vm);
  try {
    if(SQ_FAILED(sq_compilebuffer(m_vm, command.c_str(), command.length(),
                                  "", SQTrue)))
      throw SquirrelError(m_vm, "Couldn't compile command");

    sq_pushroottable(m_vm);
    if(SQ_FAILED(sq_call(m_vm, 1, SQTrue, SQTrue)))
      throw SquirrelError(m_vm, "Problem while executing command");

    if(sq_gettype(m_vm, -1) != OT_NULL)
      m_buffer.addLines(squirrel2string(m_vm, -1));
  } catch(std::exception& e) {
    m_buffer.addLines(e.what());
  }
  SQInteger newtop = sq_gettop(m_vm);
  if(newtop < oldtop) {
    log_fatal << "Script destroyed squirrel stack..." << std::endl;
  } else {
    sq_settop(m_vm, oldtop);
  }
}

void
Console::input(char c)
{
  m_inputBuffer.insert(m_inputBufferPosition, 1, c);
  m_inputBufferPosition++;
}

void
Console::backspace()
{
  if ((m_inputBufferPosition > 0) && (m_inputBuffer.length() > 0)) {
    m_inputBuffer.erase(m_inputBufferPosition-1, 1);
    m_inputBufferPosition--;
  }
}

void
Console::eraseChar()
{
  if (m_inputBufferPosition < (int)m_inputBuffer.length()) {
    m_inputBuffer.erase(m_inputBufferPosition, 1);
  }
}

void
Console::enter()
{
  m_buffer.addLines("> " + m_inputBuffer);
  parse(m_inputBuffer);
  m_inputBuffer = "";
  m_inputBufferPosition = 0;
}

void
Console::scroll(int numLines)
{
  m_offset += numLines;
  if (m_offset > 0) m_offset = 0;
}

void
Console::show_history(int offset_)
{
  while ((offset_ > 0) && (m_history_position != m_history.end())) {
    ++m_history_position;
    offset_--;
  }
  while ((offset_ < 0) && (m_history_position != m_history.begin())) {
    --m_history_position;
    offset_++;
  }
  if (m_history_position == m_history.end()) {
    m_inputBuffer = "";
    m_inputBufferPosition = 0;
  } else {
    m_inputBuffer = *m_history_position;
    m_inputBufferPosition = m_inputBuffer.length();
  }
}

void
Console::move_cursor(int offset_)
{
  if (offset_ == -65535) m_inputBufferPosition = 0;
  if (offset_ == +65535) m_inputBufferPosition = m_inputBuffer.length();
  m_inputBufferPosition+=offset_;
  if (m_inputBufferPosition < 0) m_inputBufferPosition = 0;
  if (m_inputBufferPosition > (int)m_inputBuffer.length()) m_inputBufferPosition = m_inputBuffer.length();
}

// Helper functions for Console::autocomplete
// TODO: Fix rough documentation
namespace {

void sq_insert_commands(std::list<std::string>& cmds, HSQUIRRELVM vm, const std::string& table_prefix, const std::string& search_prefix);

/**
 * Acts upon key,value on top of stack:
 * Appends key (plus type-dependent suffix) to cmds if table_prefix+key starts with search_prefix;
 * Calls sq_insert_commands if search_prefix starts with table_prefix+key (and value is a table/class/instance);
 */
void
sq_insert_command(std::list<std::string>& cmds, HSQUIRRELVM vm, const std::string& table_prefix, const std::string& search_prefix)
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
sq_insert_commands(std::list<std::string>& cmds, HSQUIRRELVM vm, const std::string& table_prefix, const std::string& search_prefix)
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
  //int autocompleteFrom = m_inputBuffer.find_last_of(" ();+", m_inputBufferPosition);
  int autocompleteFrom = m_inputBuffer.find_last_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_->.", m_inputBufferPosition);
  if (autocompleteFrom != (int)std::string::npos) {
    autocompleteFrom += 1;
  } else {
    autocompleteFrom = 0;
  }
  std::string prefix = m_inputBuffer.substr(autocompleteFrom, m_inputBufferPosition - autocompleteFrom);
  m_buffer.addLines("> " + prefix);

  std::list<std::string> cmds;

  ready_vm();

  // append all keys of the current root table to list
  sq_pushroottable(m_vm); // push root table
  while(true) {
    // check all keys (and their children) for matches
    sq_insert_commands(cmds, m_vm, "", prefix);

    // cycle through parent(delegate) table
    SQInteger oldtop = sq_gettop(m_vm);
    if(SQ_FAILED(sq_getdelegate(m_vm, -1)) || oldtop == sq_gettop(m_vm)) {
      break;
    }
    sq_remove(m_vm, -2); // remove old table
  }
  sq_pop(m_vm, 1); // remove table

  // depending on number of hits, show matches or autocomplete
  if (cmds.empty())
  {
    m_buffer.addLines("No known command starts with \"" + prefix + "\"");
  }

  if (cmds.size() == 1)
  {
    // one match: just replace input buffer with full command
    std::string replaceWith = cmds.front();
    m_inputBuffer.replace(autocompleteFrom, prefix.length(), replaceWith);
    m_inputBufferPosition += (replaceWith.length() - prefix.length());
  }

  if (cmds.size() > 1)
  {
    // multiple matches: show all matches and set input buffer to longest common prefix
    std::string commonPrefix = cmds.front();
    while (cmds.begin() != cmds.end()) {
      std::string cmd = cmds.front();
      cmds.pop_front();
      m_buffer.addLines(cmd);
      for (int n = commonPrefix.length(); n >= 1; n--) {
        if (cmd.compare(0, n, commonPrefix) != 0) commonPrefix.resize(n-1); else break;
      }
    }
    std::string replaceWith = commonPrefix;
    m_inputBuffer.replace(autocompleteFrom, prefix.length(), replaceWith);
    m_inputBufferPosition += (replaceWith.length() - prefix.length());
  }
}

void
Console::parse(const std::string& s)
{
  // make sure we actually have something to parse
  if (s.length() == 0) return;

  // add line to history
  m_history.push_back(s);
  m_history_position = m_history.end();

  // split line into list of args
  std::vector<std::string> args;
  size_t end = 0;
  while (1) {
    size_t start = s.find_first_not_of(" ,", end);
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
    m_buffer.addLines(e.what());
  }
}

bool
Console::consoleCommand(const std::string& /*command*/, const std::vector<std::string>& /*arguments*/)
{
  return false;
}

bool
Console::hasFocus() const
{
  return m_focused;
}

void
Console::show()
{
  if(!g_config->developer_mode)
    return;

  m_focused = true;
  m_height = 256;
  m_alpha = 1.0;
}

void
Console::open()
{
  if(m_stayOpen < 2)
    m_stayOpen += 1.5f;
}

void
Console::hide()
{
  m_focused = false;
  m_height = 0;
  m_stayOpen = 0;

  // clear input buffer
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
  if(m_stayOpen > 0) {
    m_stayOpen -= elapsed_time;
    if(m_stayOpen < 0)
      m_stayOpen = 0;
  } else if(!m_focused && m_height > 0) {
    m_alpha -= elapsed_time * FADE_SPEED;
    if(m_alpha < 0) {
      m_alpha = 0;
      m_height = 0;
    }
  }

  m_backgroundOffset += 600 * elapsed_time;
  if (m_backgroundOffset > (int)m_background->get_width()) m_backgroundOffset -= (int)m_background->get_width();
}

void
Console::draw(DrawingContext& context) const
{
  if (m_height == 0)
    return;

  int layer = LAYER_GUI + 1;

  context.push_transform();
  context.set_alpha(m_alpha);
  context.color().draw_surface(m_background2,
                       Vector(context.get_width()/2 - m_background->get_width()/2 - m_background->get_width() + m_backgroundOffset,
                              m_height - m_background->get_height()),
                       layer);
  context.color().draw_surface(m_background2,
                       Vector(context.get_width()/2 - m_background->get_width()/2 + m_backgroundOffset,
                              m_height - m_background->get_height()),
                       layer);
  for (int x = (context.get_width()/2 - m_background->get_width()/2
                - (static_cast<int>(ceilf((float)context.get_width() /
                                          (float)m_background->get_width()) - 1) * m_background->get_width()));
       x < context.get_width();
       x += m_background->get_width())
  {
    context.color().draw_surface(m_background, Vector(x, m_height - m_background->get_height()), layer);
  }

  int lineNo = 0;

  if (m_focused) {
    lineNo++;
    float py = m_height-4-1 * m_font->get_height();
    context.color().draw_text(m_font, "> "+m_inputBuffer, Vector(4, py), ALIGN_LEFT, layer);
    if (SDL_GetTicks() % 1000 < 750) {
      int cursor_px = 2 + m_inputBufferPosition;
      context.color().draw_text(m_font, "_", Vector(4 + (cursor_px * m_font->get_text_width("X")), py), ALIGN_LEFT, layer);
    }
  }

  int skipLines = -m_offset;
  for (std::list<std::string>::iterator i = m_buffer.m_lines.begin(); i != m_buffer.m_lines.end(); ++i)
  {
    if (skipLines-- > 0) continue;
    lineNo++;
    float py = m_height - 4 - lineNo * m_font->get_height();
    if (py < -m_font->get_height()) break;
    context.color().draw_text(m_font, *i, Vector(4, py), ALIGN_LEFT, layer);
  }
  context.pop_transform();
}

ConsoleStreamBuffer ConsoleBuffer::s_outputBuffer;
std::ostream ConsoleBuffer::output(&ConsoleBuffer::s_outputBuffer);

/* EOF */
