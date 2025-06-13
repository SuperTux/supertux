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

#ifndef HEADER_SUPERTUX_CONTROL_ONLINE_CONTROLLER_HPP
#define HEADER_SUPERTUX_CONTROL_ONLINE_CONTROLLER_HPP

#include "control/controller.hpp"

class OnlineController final : public Controller
{
public:
  OnlineController();
  ~OnlineController() override;

  void press(Control c);

  void release(Control c);

  virtual void update() override
  {
  }

  void clear_controls();

private:
  OnlineController(const OnlineController&) = delete;
  OnlineController& operator=(const OnlineController&) = delete;
};

#endif

/* EOF */
