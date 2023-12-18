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

#include <fmt/format.h>

#include "math/sizef.hpp"
#include "physfs/ifile_stream.hpp"
#include "squirrel/squirrel_virtual_machine.hpp"
#include "squirrel/squirrel_util.hpp"
#include "squirrel/suggestions.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "supertux/resources.hpp"
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
  assert((console && !m_console) ||
         (m_console && !console));

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

  // Output line to stderr.
  get_logging_instance(false) << s << std::endl;

  // Wrap long lines.
  std::string overflow;
  int line_count = 0;
  do {
    m_lines.push_front(Font::wrap_to_chars(s, 99, &overflow));
    line_count += 1;
    s = overflow;
  } while (s.length() > 0);

  // Trim scrollback buffer.
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
  m_background(Surface::from_file("images/engine/console.png")),
  m_background2(Surface::from_file("images/engine/console2.png")),
  m_vm(nullptr),
  m_vm_object(),
  m_backgroundOffset(0),
  m_height(0),
  m_alpha(1.0),
  m_offset(0),
  m_focused(false),
  m_font(Resources::console_font),
  m_stayOpen(0)
{
  m_buffer.set_console(this);
}

Console::~Console()
{
  if (m_vm != nullptr && SquirrelVirtualMachine::current() != nullptr)
  {
    sq_release(SquirrelVirtualMachine::current()->get_vm().get_vm(), &m_vm_object);
  }
  m_buffer.set_console(nullptr);
}

void
Console::on_buffer_change(int line_count)
{
  // Increase console height if necessary.
  if (m_stayOpen > 0 && m_height < 64)
  {
    if (m_height < 4)
    {
      m_height = 4;
    }
    m_height += m_font->get_height() * static_cast<float>(line_count);
  }

  // Reset console to full opacity.
  m_alpha = 1.0;
}

void
Console::ready_vm()
{
  if (m_vm == nullptr) {
    m_vm = SquirrelVirtualMachine::current()->get_vm().get_vm();
    HSQUIRRELVM new_vm = sq_newthread(m_vm, 16);
    if (new_vm == nullptr)
      throw SquirrelError(m_vm, "Couldn't create new VM thread for console");

    // Store reference to thread.
    sq_resetobject(&m_vm_object);
    if (SQ_FAILED(sq_getstackobj(m_vm, -1, &m_vm_object)))
      throw SquirrelError(m_vm, "Couldn't get vm object for console");
    sq_addref(m_vm, &m_vm_object);
    sq_pop(m_vm, 1);

    // Create new roottable for thread.
    sq_newtable(new_vm);
    sq_pushroottable(new_vm);
    if (SQ_FAILED(sq_setdelegate(new_vm, -2)))
      throw SquirrelError(new_vm, "Couldn't set console_table delegate");

    sq_setroottable(new_vm);

    m_vm = new_vm;

    try {
      std::string filename = "scripts/console.nut";
      IFileStream stream(filename);
      compile_and_run(m_vm, stream, filename);
    } catch(std::exception& e) {
      log_warning << "Couldn't load console.nut: " << e.what() << std::endl;
    }
  }
}

void
Console::execute_script(const std::string& command)
{
  ready_vm();

  SQInteger oldtop = sq_gettop(m_vm);
  try {
    if (SQ_FAILED(sq_compilebuffer(m_vm, command.c_str(), command.length(),
                                  "", SQTrue)))
      throw SquirrelError(m_vm, "Couldn't compile command");

    sq_pushroottable(m_vm);
    if (SQ_FAILED(sq_call(m_vm, 1, SQTrue, SQTrue)))
      throw SquirrelError(m_vm, "Problem while executing command");

    if (sq_gettype(m_vm, -1) != OT_NULL)
      m_buffer.addLines(squirrel2string(m_vm, -1));
  } catch(std::exception& e) {
    m_buffer.addLines(e.what());
  }
  SQInteger newtop = sq_gettop(m_vm);
  if (newtop < oldtop) {
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
  if (m_inputBufferPosition < static_cast<int>(m_inputBuffer.length())) {
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
    m_inputBufferPosition = static_cast<int>(m_inputBuffer.length());
  }
}

void
Console::move_cursor(int offset_)
{
  if (offset_ == -65535) m_inputBufferPosition = 0;
  if (offset_ == +65535) m_inputBufferPosition = static_cast<int>(m_inputBuffer.length());
  m_inputBufferPosition+=offset_;
  if (m_inputBufferPosition < 0) m_inputBufferPosition = 0;
  if (m_inputBufferPosition > static_cast<int>(m_inputBuffer.length())) m_inputBufferPosition = static_cast<int>(m_inputBuffer.length());
}

void
Console::autocomplete()
{
  // int autocompleteFrom = m_inputBuffer.find_last_of(" ();+", m_inputBufferPosition);
  int autocompleteFrom = static_cast<int>(m_inputBuffer.find_last_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_->.", m_inputBufferPosition));
  if (autocompleteFrom != static_cast<int>(std::string::npos)) {
    autocompleteFrom += 1;
  } else {
    autocompleteFrom = 0;
  }
  const std::string prefix = m_inputBuffer.substr(autocompleteFrom, m_inputBufferPosition - autocompleteFrom);
  m_buffer.addLines("> " + prefix);

  ready_vm();
  const squirrel::SuggestionStack cmds = squirrel::get_suggestions(prefix, false);

  // Depending on number of hits, show matches or autocomplete.
  m_buffer.addLine(""); // Additional spacing line
  if (cmds.empty())
  {
    m_buffer.addLines("No known command starts with \"" + prefix + "\"");
    m_buffer.addLine(""); // End spacing line
  }
  else if (cmds.size() == 1)
  {
    // One match: just replace input buffer with full command.
    const std::string& replaceWith = cmds.begin()->name;
    m_inputBuffer.replace(autocompleteFrom, prefix.length(), replaceWith);
    m_inputBufferPosition += static_cast<int>(replaceWith.length() - prefix.length());

    print_suggestion(cmds.back());
    m_buffer.addLine(""); // End spacing line
  }
  else if (cmds.size() > 1)
  {
    // Multiple matches: show all matches and set input buffer to longest common prefix.
    std::string commonPrefix = cmds.begin()->name;
    for (auto i = cmds.begin(); i != cmds.end(); i++)
    {
      print_suggestion(*i);
      m_buffer.addLine(""); // Spacing line
      for (int n = static_cast<int>(commonPrefix.length()); n >= 1; n--) {
        if (i->name.compare(0, n, commonPrefix) != 0) commonPrefix.resize(n-1); else break;
      }
    }
    std::string replaceWith = commonPrefix;
    m_inputBuffer.replace(autocompleteFrom, prefix.length(), replaceWith);
    m_inputBufferPosition += static_cast<int>(replaceWith.length() - prefix.length());
  }
}

void
Console::print_suggestion(const squirrel::Suggestion& suggestion)
{
  if (!suggestion.reference)
  {
    m_buffer.addLine("\"" + suggestion.name + "\"");
    return;
  }

  using namespace squirrel;

  switch (suggestion.reference->get_type())
  {
    case ScriptingObject::Type::CONSTANT:
    {
      const auto& con = static_cast<const ScriptingConstant&>(*suggestion.reference);
      m_buffer.addLine("\"" + con.type + " " + con.name + "\" - " + con.description);
    }
    break;

    case ScriptingObject::Type::FUNCTION:
    {
      const auto& func = static_cast<const ScriptingFunction&>(*suggestion.reference);

      std::stringstream out;
      out << func.type << " " << func.name << "(";
      for (size_t i = 0; i < func.parameters.size(); i++)
      {
        const auto& param = func.parameters.at(i);

        out << param.type << " " << param.name;
        if (i != func.parameters.size() - 1)
          out << ", ";
      }
      out << ")";

      m_buffer.addLine("\"" + out.str() + "\" - " + func.description);
    }
    break;

    case ScriptingObject::Type::CLASS:
    {
      const auto& cl = static_cast<const ScriptingClass&>(*suggestion.reference);
      m_buffer.addLine("\"" + suggestion.name + "\" - " +
        (suggestion.is_instance ? fmt::format(fmt::runtime(_("Instance of `{}`.")), cl.name) + " " : "") +
        cl.summary);
    }
    break;
  }
}

void
Console::parse(const std::string& s)
{
  // Make sure we actually have something to parse.
  if (s.length() == 0) return;

  // Add line to history.
  m_history.push_back(s);
  m_history_position = m_history.end();

  // Split line into list of args.
  std::vector<std::string> args;
  size_t end = 0;
  while (1) {
    size_t start = s.find_first_not_of(" ,", end);
    end = s.find_first_of(" ,", start);
    if (start == s.npos) break;
    args.push_back(s.substr(start, end-start));
  }

  // Command is args[0].
  if (args.size() == 0) return;
  std::string command = args.front();
  args.erase(args.begin());

  // Ignore if it's an internal command.
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
  if (!g_config->developer_mode)
    return;

  m_focused = true;
  m_height = 256;
  m_alpha = 1.0;
}

void
Console::open()
{
  if (m_stayOpen < 2)
    m_stayOpen += 1.5f;
}

void
Console::hide()
{
  m_focused = false;
  m_height = 0;
  m_stayOpen = 0;

  // Clear the input buffer.
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
Console::update(float dt_sec)
{
  if (m_stayOpen > 0) {
    m_stayOpen -= dt_sec;
    if (m_stayOpen < 0)
      m_stayOpen = 0;
  } else if (!m_focused && m_height > 0) {
    m_alpha -= dt_sec * FADE_SPEED;
    if (m_alpha < 0) {
      m_alpha = 0;
      m_height = 0;
    }
  }

  m_backgroundOffset += static_cast<int>(600.0f * dt_sec);
  if (m_backgroundOffset > static_cast<int>(m_background->get_width())) m_backgroundOffset -= static_cast<int>(m_background->get_width());
}

void
Console::draw(DrawingContext& context) const
{
  if (m_height == 0)
    return;

  const int layer = LAYER_GUI + 1;
  const float context_center_x = context.get_width() / 2;
  const int background_center_x = m_background->get_width() / 2;

  context.push_transform();
  context.set_alpha(m_alpha);
  context.color().draw_surface(m_background2,
                               Vector(context_center_x - static_cast<float>(background_center_x + m_background->get_width() - m_backgroundOffset),
                                      m_height - static_cast<float>(m_background->get_height())),
                               layer);
  context.color().draw_surface(m_background2,
                               Vector(context_center_x - static_cast<float>(background_center_x - m_backgroundOffset),
                                      m_height - static_cast<float>(m_background->get_height())),
                               layer);
  for (int x = (static_cast<int>(context_center_x) - background_center_x
                - (static_cast<int>(ceilf(context.get_width() /
                                          static_cast<float>(m_background->get_width())) - 1) * m_background->get_width()));
       x < static_cast<int>(context.get_width());
       x += m_background->get_width())
  {
    context.color().draw_surface(m_background, Vector(static_cast<float>(x),
                                                      m_height - static_cast<float>(m_background->get_height())),
                                 layer);
  }

  int lineNo = 0;

  if (m_focused) {
    lineNo++;
    float py = m_height-4-1 * m_font->get_height();
    std::string line = "> " + m_inputBuffer;
    context.color().draw_text(m_font, line, Vector(4, py), ALIGN_LEFT, layer);

    if (SDL_GetTicks() % 500 < 250) {
      std::string::size_type p = 2 + m_inputBufferPosition;
      float cursor_x;
      if (p >= line.size())
      {
        cursor_x = m_font->get_text_width(line);
      }
      else
      {
        cursor_x = m_font->get_text_width(line.substr(0, p));
      }
      context.color().draw_filled_rect(Rectf(Vector(3 + cursor_x, py),
                                             Sizef(2.0f, m_font->get_height() - 2)),
                                       Color(1.0f, 1.0f, 1.0f, 0.75f), layer);
    }
  }

  int skipLines = -m_offset;
  for (std::list<std::string>::iterator i = m_buffer.m_lines.begin(); i != m_buffer.m_lines.end(); ++i)
  {
    if (skipLines-- > 0) continue;
    lineNo++;
    float py = static_cast<float>(m_height - 4.0f - static_cast<float>(lineNo) * m_font->get_height());
    if (py < -m_font->get_height()) break;
    context.color().draw_text(m_font, *i, Vector(4.0f, py), ALIGN_LEFT, layer);
  }
  context.pop_transform();
}

ConsoleStreamBuffer ConsoleBuffer::s_outputBuffer;
std::ostream ConsoleBuffer::output(&ConsoleBuffer::s_outputBuffer);

/* EOF */
