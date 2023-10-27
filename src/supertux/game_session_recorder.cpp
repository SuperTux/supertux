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
#include "math/random.hpp"
#include "object/player.hpp"
#include "supertux/game_session.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "supertux/sector.hpp"
#include "util/log.hpp"

GameSessionRecorder::GameSessionRecorder() :
  m_capture_file(),
  m_capture_demo_stream(),
  m_playback_demo_stream(),
  m_demo_controller(),
  m_playing(false)
{
}

GameSessionRecorder::~GameSessionRecorder()
{
}

void
GameSessionRecorder::start_recording()
{
  if (!m_capture_file.empty()) {
    int newSeed = 0;               // Next run uses a new seed
    while (newSeed == 0)            // which is the next non-zero random num.
      newSeed = gameRandom.rand();
    g_config->random_seed = newSeed;
    gameRandom.seed(g_config->random_seed);
    log_info << "Next run uses random seed " << g_config->random_seed <<std::endl;
    record_demo(m_capture_file);
  }
}

void
GameSessionRecorder::record_demo(const std::string& filename)
{
  m_capture_demo_stream.reset(new std::ofstream(filename.c_str()));
  if (!m_capture_demo_stream->good()) {
    std::stringstream msg;
    msg << "Couldn't open demo file '" << filename << "' for writing.";
    throw std::runtime_error(msg.str());
  }
  m_capture_file = filename;

  char buf[30];                            // Save the seed in the demo file.
  snprintf(buf, sizeof(buf), "random_seed=%10d", g_config->random_seed);
  for (int i = 0; i == 0 || buf[i-1]; i++)
    m_capture_demo_stream->put(buf[i]);
}

int
GameSessionRecorder::get_demo_random_seed(const std::string& filename) const
{
  std::unique_ptr<std::istream> test_stream(new std::ifstream(filename.c_str()));
  if (test_stream->good())
  {
    char buf[30];                     // Recall the seed from the demo file.
    int seed;

    for (int i=0; i<30 && (i==0 || buf[i-1]); i++)
      test_stream->get(buf[i]);

    if (sscanf(buf, "random_seed=%10d", &seed) == 1)
    {
      log_info << "Random seed " << seed << " from demo file." << std::endl;
      return seed;
    }
    else
    {
      log_info << "Demo file contains no random number." << std::endl;
    }
  }
  return 0;
}

void
GameSessionRecorder::play_demo(const std::string& filename)
{
  m_playing = true;

  m_playback_demo_stream.reset();
  m_demo_controller.reset();

  m_playback_demo_stream.reset(new std::ifstream(filename.c_str()));
  if (!m_playback_demo_stream->good()) {
    std::stringstream msg;
    msg << "Couldn't open demo file '" << filename << "' for reading.";
    throw std::runtime_error(msg.str());
  }

  reset_demo_controller();

  // Skip over random seed, if it exists in the file.
  char buf[30];                            // Ascii decimal seed.
  int seed;
  for (int i=0; i<30 && (i==0 || buf[i-1]); i++)
    m_playback_demo_stream->get(buf[i]);
  if (sscanf(buf, "random_seed=%010d", &seed) != 1)
    m_playback_demo_stream->seekg(0);     // Old style w/o seed, restart at beg.

  m_playing = false;
}

void
GameSessionRecorder::reset_demo_controller()
{
  if (!m_demo_controller) {
    m_demo_controller.reset(new CodeController());
  }

  auto game_session = GameSession::current();
  assert(game_session != nullptr);
  // FIXME: How is this going to be handled?
  Player& player = *(game_session->get_current_sector().get_players()[0]);
  player.set_controller(m_demo_controller.get());
}

void
GameSessionRecorder::process_events()
{
  // Playback a demo?
  if (m_playback_demo_stream != nullptr)
  {
    m_demo_controller->update();

    char left, right, up, down, jump, action;

    m_playback_demo_stream->get(left);
    m_playback_demo_stream->get(right);
    m_playback_demo_stream->get(up);
    m_playback_demo_stream->get(down);
    m_playback_demo_stream->get(jump);
    m_playback_demo_stream->get(action);

    m_demo_controller->press(Control::LEFT, left != 0);
    m_demo_controller->press(Control::RIGHT, right != 0);
    m_demo_controller->press(Control::UP, up != 0);
    m_demo_controller->press(Control::DOWN,  down != 0);
    m_demo_controller->press(Control::JUMP, jump != 0);
    m_demo_controller->press(Control::ACTION,  action != 0);
  }

  // Save input for demo?
  if (m_capture_demo_stream != nullptr)
  {
    Controller& controller = InputManager::current()->get_controller();

    m_capture_demo_stream->put(controller.hold(Control::LEFT));
    m_capture_demo_stream->put(controller.hold(Control::RIGHT));
    m_capture_demo_stream->put(controller.hold(Control::UP));
    m_capture_demo_stream->put(controller.hold(Control::DOWN));
    m_capture_demo_stream->put(controller.hold(Control::JUMP));
    m_capture_demo_stream->put(controller.hold(Control::ACTION));
  }
}

/* EOF */
