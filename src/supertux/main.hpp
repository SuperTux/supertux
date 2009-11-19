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

class Main
{
private:
  void init_config();
  void init_tinygettext();
  void init_physfs(const char* argv0);
  void print_usage(const char* argv0);
  bool parse_commandline(int argc, char** argv);
  void init_sdl();
  void init_rand();
  void init_audio();
  void quit_audio();
  bool pre_parse_commandline(int argc, char** argv);

public:
  int  main(int argc, char** argv);
  void init_video();
  void wait_for_event(float min_delay, float max_delay);
};

#endif

/* EOF */
