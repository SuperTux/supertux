//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#ifndef HEADER_SUPERTUX_SUPERTUX_MAIN_HPP
#define HEADER_SUPERTUX_SUPERTUX_MAIN_HPP

#include <string>

class CommandLineArguments;

class Main final
{
public:
  Main();

  /** We call it run() instead of main() as main collides with
      #define main SDL_main from SDL.h */
  int run(int argc, char** argv);

private:
  void init_tinygettext();
  void init_video();

  void launch_game(const CommandLineArguments& args);
  void launch_launcher(const CommandLineArguments& args, bool& launch_game_on_exit);
  void resave(const std::string& input_filename, const std::string& output_filename);

private:
  Main(const Main&) = delete;
  Main& operator=(const Main&) = delete;
};

#endif

/* EOF */
