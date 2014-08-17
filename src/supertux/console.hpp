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

#ifndef HEADER_SUPERTUX_SUPERTUX_CONSOLE_HPP
#define HEADER_SUPERTUX_SUPERTUX_CONSOLE_HPP

#include <list>
#include <memory>
#include <squirrel.h>
#include <sstream>
#include <vector>

#include "util/currenton.hpp"
#include "video/font_ptr.hpp"
#include "video/surface_ptr.hpp"

class Console;
class ConsoleStreamBuffer;
class ConsoleCommandReceiver;
class DrawingContext;

class Console : public Currenton<Console>
{
public:
  Console();
  ~Console();

  static std::ostream output; /**< stream of characters to output to the console. Do not forget to send std::endl or to flush the stream. */

  void init_graphics();

  void input(char c); /**< add character to inputBuffer */
  void backspace(); /**< delete character left of inputBufferPosition */
  void eraseChar(); /**< delete character at inputBufferPosition */
  void enter(); /**< process and clear input stream */
  void scroll(int offset); /**< scroll console text up or down by @c offset lines */
  void autocomplete(); /**< autocomplete current command */
  void show_history(int offset); /**< move @c offset lines forward through history; Negative offset moves backward */
  void move_cursor(int offset); /**< move the cursor @c offset chars to the right; Negative offset moves backward; 0xFFFF moves to the end */

  void draw(DrawingContext& context); /**< draw the console in a DrawingContext */
  void update(float elapsed_time);

  void show(); /**< display the console */
  void open(); /**< open the console for viewing for 6 seconds */
  void hide(); /**< hide the console */
  void toggle(); /**< display the console if hidden, hide otherwise */

  bool hasFocus(); /**< true if characters should be sent to the console instead of their normal target */

private:
  std::list<std::string> history; /**< command history. New lines get added to back. */
  std::list<std::string>::iterator history_position; /**< item of command history that is currently displayed */
  std::list<std::string> lines; /**< backbuffer of lines sent to the console. New lines get added to front. */

  SurfacePtr background; /**< console background image */
  SurfacePtr background2; /**< second, moving console background image */

  HSQUIRRELVM vm; /**< squirrel thread for the console (with custom roottable) */
  HSQOBJECT vm_object;

  int backgroundOffset; /**< current offset of scrolling background image */
  float height; /**< height of the console in px */
  float alpha;
  int offset; /**< decrease to scroll text up */
  bool focused; /**< true if console has input focus */
  FontPtr font;
  float fontheight; /**< height of the font (this is a separate var, because the font could not be initialized yet but is needed in the addLine message */

  float stayOpen;

  static int inputBufferPosition; /**< position in inputBuffer before which to append new characters */
  static std::string inputBuffer; /**< string used for keyboard input */
  static ConsoleStreamBuffer outputBuffer; /**< stream buffer used by output stream */

  void addLines(std::string s); /**< display a string of (potentially) multiple lines in the console */
  void addLine(std::string s); /**< display a line in the console */
  void parse(std::string s); /**< react to a given command */

  /** ready a virtual machine instance, creating a new thread and loading default .nut files if needed */
  void ready_vm();

  /** execute squirrel script and output result */
  void execute_script(const std::string& s);

  bool consoleCommand(std::string command, std::vector<std::string> arguments); /**< process internal command; return false if command was unknown, true otherwise */

  friend class ConsoleStreamBuffer;
  void flush(ConsoleStreamBuffer* buffer); /**< act upon changes in a ConsoleStreamBuffer */

private:
  Console(const Console&);
  Console & operator=(const Console&);
};

class ConsoleStreamBuffer : public std::stringbuf
{
public:
  int sync()
  {
    int result = std::stringbuf::sync();
    if(Console::current())
      Console::current()->flush(this);
    return result;
  }
};

#endif

/* EOF */
