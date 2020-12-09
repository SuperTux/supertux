//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//  Copyright (C) 2020 A. Semphris <semphris@protonmail.com>
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

#ifndef HEADER_SUPERTUX_LAUNCHER_MAIN_HPP
#define HEADER_SUPERTUX_LAUNCHER_MAIN_HPP

#include <string>

class CommandLineArguments;

class LauncherMain final
{
public:
  LauncherMain();

  int run(int argc, char** argv);

private:
  void init_tinygettext();
  void init_video();

  void launch_game(const CommandLineArguments& args);
  void resave(const std::string& input_filename, const std::string& output_filename);

private:
  char* arg0;

private:
  LauncherMain(const LauncherMain&) = delete;
  LauncherMain& operator=(const LauncherMain&) = delete;
};

#endif

/* EOF */
