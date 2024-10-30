//  SuperTux
//  Copyright (C) 2024 Vankata453
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

#ifndef HEADER_SUPERTUX_CONTROL_NETWORK_CONTROLLER_HPP
#define HEADER_SUPERTUX_CONTROL_NETWORK_CONTROLLER_HPP

#include "control/controller.hpp"

#include <vector>

namespace network {
class ReceivedPacket;
} // namespace network

/** This is a dummy controller that doesn't react to any user input but should
    be controlled by network packets */
class NetworkController final : public Controller
{
public:
  NetworkController();
  ~NetworkController() override;

  virtual void update() override;

  void process_packet_data(const network::ReceivedPacket& packet, int start_idx);

private:
  typedef std::array<bool, static_cast<int>(Control::CONTROLCOUNT)> ControlArray;

private:
  std::vector<ControlArray> m_controls_buffer;

private:
  NetworkController(const NetworkController&) = delete;
  NetworkController& operator=(const NetworkController&) = delete;
};

#endif

/* EOF */
