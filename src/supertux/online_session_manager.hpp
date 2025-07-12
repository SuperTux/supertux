//  SuperTux
//  Copyright (C) 2025 Martim Ferreira <martim.silva.ferreira@tecnico.ulisboa.pt>
//                2025 Gonçalo Rocha <goncalo.r.f.rocha@tecnico.ulisboa.pt>
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

#ifndef HEADER_SUPERTUX_SUPERTUX_ONLINE_SESSION_MANAGER_HPP
#define HEADER_SUPERTUX_SUPERTUX_ONLINE_SESSION_MANAGER_HPP

#include "util/currenton.hpp"

#include <condition_variable>
#include <cstring>
#include <mutex>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>

class OnlineSessionManager final : public Currenton<OnlineSessionManager>
{
public:
  OnlineSessionManager();
  ~OnlineSessionManager();

  void start_thread();

  void stop_thread();

  void receive_messages();

  void send_messages();

private:
  bool m_running;
  std::thread m_receive_thread;
  std::thread m_send_thread;
  std::mutex m_mutex;
  std::condition_variable m_cv;
  int m_server_socket;

private:
  OnlineSessionManager(const OnlineSessionManager&) = delete;
  OnlineSessionManager& operator=(const OnlineSessionManager&) = delete;  
};

#endif

/* EOF */
