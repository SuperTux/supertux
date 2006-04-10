//  $Id: worldmap.hpp 3209 2006-04-02 22:19:22Z sommer $
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

#ifndef SUPERTUX_CONSOLE_H
#define SUPERTUX_CONSOLE_H

#include <list>
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>

class Console;
class ConsoleStreamBuffer;
class ConsoleCommandReceiver;
class DrawingContext;
class Surface;

class Console 
{
  public:
    Console();
    ~Console();

    static std::ostream input; /**< stream of keyboard input to send to the console. Do not forget to send std::endl or to flush the stream. */
    static std::ostream output; /**< stream of characters to output to the console. Do not forget to send std::endl or to flush the stream. */

    static void backspace(); /**< delete last character sent to the input stream */
    static void scroll(int offset); /**< scroll console text up or down by @c offset lines */
    static void autocomplete(); /**< autocomplete current command */

    void draw(DrawingContext& context); /**< draw the console in a DrawingContext */
    static void show(); /**< display the console */
    static void hide(); /**< hide the console */
    static void toggle(); /**< display the console if hidden, hide otherwise */

    static bool hasFocus(); /**< true if characters should be sent to the console instead of their normal target */
    static void registerCommand(std::string command, ConsoleCommandReceiver* ccr); /**< associate command with the given CCR */
    static void unregisterCommand(std::string command, ConsoleCommandReceiver* ccr); /**< dissociate command and CCR */
    static void unregisterCommands(ConsoleCommandReceiver* ccr); /**< dissociate all commands of given CCR */

    template<typename T> static bool string_is(std::string s) {
      std::istringstream iss(s);
      T i;
      if ((iss >> i) && iss.eof()) {
	return true;
      } else {
	return false;
      }
    }

    template<typename T> static T string_to(std::string s) {
      std::istringstream iss(s);
      T i;
      if ((iss >> i) && iss.eof()) {
	return i;
      } else {
	return T();
      }
    }

  protected:
    static std::list<std::string> lines; /**< backbuffer of lines sent to the console */
    static std::map<std::string, std::list<ConsoleCommandReceiver*> > commands; /**< map of console commands and a list of associated ConsoleCommandReceivers */
    Surface* background; /**< console background image */
    Surface* background2; /**< second, moving console background image */
    static int backgroundOffset; /**< current offset of scrolling background image */
    static int height; /**< height of the console in px */
    static int offset; /**< decrease to scroll text up */
    static bool focused; /**< true if console has input focus */

    static ConsoleStreamBuffer inputBuffer; /**< stream buffer used by input stream */
    static ConsoleStreamBuffer outputBuffer; /**< stream buffer used by output stream */

    static void addLine(std::string s); /**< display a line in the console */
    static void parse(std::string s); /**< react to a given command */
    
    /** execute squirrel script and output result */
    static void execute_script(const std::string& s);
    
    static bool consoleCommand(std::string command, std::vector<std::string> arguments); /**< process internal command; return false if command was unknown, true otherwise */

    friend class ConsoleStreamBuffer;
    static void flush(ConsoleStreamBuffer* buffer); /**< act upon changes in a ConsoleStreamBuffer */
};

class ConsoleStreamBuffer : public std::stringbuf 
{
  public:
    int sync() 
    {
      int result = std::stringbuf::sync();
      Console::flush(this);
      return result;
    }
};

class ConsoleCommandReceiver
{
public:
  virtual ~ConsoleCommandReceiver()
  {
    Console::unregisterCommands(this);
  }
   
  /**
   * callback from Console; return false if command was unknown,
   * true otherwise
   */
  virtual bool consoleCommand(std::string command, std::vector<std::string> arguments) = 0;
};

#endif
