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

#include "control/network_controller.hpp"

#include <stdexcept>

#include "network/packet.hpp"

NetworkController::NetworkController() :
  m_controls_buffer()
{
}

NetworkController::~NetworkController()
{
}

void
NetworkController::process_packet_data(const network::ReceivedPacket& packet, int start_idx)
{
  ControlArray controls{};

  // All pressed controls are included.
  for (int i = start_idx; i < static_cast<int>(packet.data.size()); i++)
  {
    const int control = std::stoi(packet.data[i]);
    if (control >= static_cast<int>(Control::CONTROLCOUNT))
      throw std::runtime_error("Invalid control ID " + packet.data[i]);

    controls[control] = true;
  }

  m_controls_buffer.push_back(std::move(controls));
}

void
NetworkController::update()
{
  Controller::update();

  ControlArray controls{};
  if (!m_controls_buffer.empty())
  {
    controls = m_controls_buffer.front();
    m_controls_buffer.erase(m_controls_buffer.begin());
  }

  for (int i = 0; i < static_cast<int>(Control::CONTROLCOUNT); ++i)
  {
    m_controls[i] = controls[i];
  }
}

/* EOF */
