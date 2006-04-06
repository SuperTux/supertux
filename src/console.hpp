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
#include <string>
#include <sstream>
#include <iostream>

class Console;
class ConsoleStreamBuffer;
class DrawingContext;
class Surface;

class Console 
{
  public:
    Console(DrawingContext* context);
    ~Console();

    static std::ostream input; /**< stream of keyboard input to send to the console. Do not forget to send std::endl or to flush the stream. */
    static std::ostream output; /**< stream of characters to output to the console. Do not forget to send std::endl or to flush the stream. */

    static void flush(ConsoleStreamBuffer* buffer); /**< act upon changes in a stream, normally called by the stream itself */

    void draw(); /**< draw the console to its */
    static void show(); /**< display the console */
    static void hide(); /**< hide the console */
    static bool hasFocus(); /**< true if characters should be sent to the console instead of their normal target */

  protected:
    static std::list<std::string> lines; /**< backbuffer of lines sent to the console */
    DrawingContext* context; /**< context to draw to */
    Surface* background; /**< console background image */
    static int height; /**< height of the console in px */
    static bool focused; /**< true if console has input focus */

    static ConsoleStreamBuffer inputBuffer; /**< stream buffer used by input stream */
    static ConsoleStreamBuffer outputBuffer; /**< stream buffer used by output stream */

    static void addLine(std::string s); /**< display a line in the console */
    static void parse(std::string s); /**< react to a given command */
};

class ConsoleStreamBuffer : public std::stringbuf 
{
  public:
    int sync() 
    {
      Console::flush(this);
      return std::stringbuf::sync();
    }
};

#endif


