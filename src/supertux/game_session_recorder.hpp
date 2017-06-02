//  SuperTux
//  Copyright (C) 2017 Tobias Markus <tobbi.bugs@googlemail.com>
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

#ifndef HEADER_SUPERTUX_SUPERTUX_GAME_SESSION_RECORDER_HPP
#define HEADER_SUPERTUX_SUPERTUX_GAME_SESSION_RECORDER_HPP

#include <string>

#include "control/codecontroller.hpp"

class GameSessionRecorder
{
public:
  GameSessionRecorder();
  virtual ~GameSessionRecorder();

  void start_recording();
  void record_demo(const std::string& filename);
  int get_demo_random_seed(const std::string& filename) const;
  void play_demo(const std::string& filename);
  void process_events();

private:
  void capture_demo_step();

  std::ostream* capture_demo_stream;
  std::string capture_file;
  std::istream* playback_demo_stream;
  CodeController* demo_controller;

private:
  GameSessionRecorder(const GameSessionRecorder&) = delete;
  GameSessionRecorder& operator=(const GameSessionRecorder&) = delete;
};

#endif

