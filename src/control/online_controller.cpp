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

#include "control/online_controller.hpp"

OnlineController::OnlineController()
{
}

OnlineController::~OnlineController()
{
}

void
OnlineController::press(Control c)
{
  m_controls[static_cast<int>(c)] = true;
  m_last_input += ":" + std::to_string(static_cast<int>(c));
}

void
OnlineController::release(Control c)
{
  m_controls[static_cast<int>(c)] = false;
}

void
OnlineController::clear_controls()
{
  Controller::update();

  for (int i = 0; i < static_cast<int>(Control::CONTROLCOUNT); ++i) {
    m_controls[i] = false;
  }
}

/* EOF */
