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

#include "supertux/game_session_recorder.hpp"

#include <fstream>

#include "control/input_manager.hpp"
#include "math/random_generator.hpp"
#include "object/player.hpp"
#include "supertux/game_session.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "supertux/sector.hpp"
#include "util/log.hpp"

GameSessionRecorder::GameSessionRecorder() :
  capture_demo_stream(0),
  capture_file(),
  playback_demo_stream(0),
  demo_controller(0)
{
}

GameSessionRecorder::~GameSessionRecorder()
{
  delete capture_demo_stream;
  delete playback_demo_stream;
  delete demo_controller;
}

void
GameSessionRecorder::start_recording()
{
  if(!capture_file.empty()) {
    int newSeed = 0;               // next run uses a new seed
    while (newSeed == 0)            // which is the next non-zero random num.
      newSeed = gameRandom.rand();
    g_config->random_seed = gameRandom.srand(newSeed);
    log_info << "Next run uses random seed " << g_config->random_seed <<std::endl;
    record_demo(capture_file);
  }
}

void
GameSessionRecorder::record_demo(const std::string& filename)
{
  delete capture_demo_stream;

  capture_demo_stream = new std::ofstream(filename.c_str());
  if(!capture_demo_stream->good()) {
    std::stringstream msg;
    msg << "Couldn't open demo file '" << filename << "' for writing.";
    throw std::runtime_error(msg.str());
  }
  capture_file = filename;

  char buf[30];                            // save the seed in the demo file
  snprintf(buf, sizeof(buf), "random_seed=%10d", g_config->random_seed);
  for (int i = 0; i == 0 || buf[i-1]; i++)
    capture_demo_stream->put(buf[i]);
}

int
GameSessionRecorder::get_demo_random_seed(const std::string& filename) const
{
  std::istream* test_stream = new std::ifstream(filename.c_str());
  if(test_stream->good()) {
    char buf[30];                     // recall the seed from the demo file
    int seed;
    for (int i=0; i<30 && (i==0 || buf[i-1]); i++)
      test_stream->get(buf[i]);
    if (sscanf(buf, "random_seed=%10d", &seed) == 1) {
      log_info << "Random seed " << seed << " from demo file" << std::endl;
      delete test_stream;
      test_stream = nullptr;
      return seed;
    }
    else
      log_info << "Demo file contains no random number" << std::endl;
  }
  delete test_stream;
  test_stream = nullptr;
  return 0;
}

void
GameSessionRecorder::play_demo(const std::string& filename)
{
  delete playback_demo_stream;
  delete demo_controller;

  playback_demo_stream = new std::ifstream(filename.c_str());
  if(!playback_demo_stream->good()) {
    std::stringstream msg;
    msg << "Couldn't open demo file '" << filename << "' for reading.";
    throw std::runtime_error(msg.str());
  }

  Player& tux = *GameSession::current()->get_current_sector()->player;
  demo_controller = new CodeController();
  tux.set_controller(demo_controller);

  // skip over random seed, if it exists in the file
  char buf[30];                            // ascii decimal seed
  int seed;
  for (int i=0; i<30 && (i==0 || buf[i-1]); i++)
    playback_demo_stream->get(buf[i]);
  if (sscanf(buf, "random_seed=%010d", &seed) != 1)
    playback_demo_stream->seekg(0);     // old style w/o seed, restart at beg
}

void
GameSessionRecorder::process_events()
{
  // playback a demo?
  if(playback_demo_stream != 0) {
    demo_controller->update();
    char left = false;
    char right = false;
    char up = false;
    char down = false;
    char jump = false;
    char action = false;
    playback_demo_stream->get(left);
    playback_demo_stream->get(right);
    playback_demo_stream->get(up);
    playback_demo_stream->get(down);
    playback_demo_stream->get(jump);
    playback_demo_stream->get(action);
    demo_controller->press(Controller::LEFT, left);
    demo_controller->press(Controller::RIGHT, right);
    demo_controller->press(Controller::UP, up);
    demo_controller->press(Controller::DOWN, down);
    demo_controller->press(Controller::JUMP, jump);
    demo_controller->press(Controller::ACTION, action);
  }

  // save input for demo?
  if(capture_demo_stream != 0) {
    auto controller = InputManager::current()->get_controller();
    capture_demo_stream ->put(controller->hold(Controller::LEFT));
    capture_demo_stream ->put(controller->hold(Controller::RIGHT));
    capture_demo_stream ->put(controller->hold(Controller::UP));
    capture_demo_stream ->put(controller->hold(Controller::DOWN));
    capture_demo_stream ->put(controller->hold(Controller::JUMP));
    capture_demo_stream ->put(controller->hold(Controller::ACTION));
  }
}

