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

#include "control/controller.hpp"

const char* Controller::controlNames[] = {
  "left",
  "right",
  "up",
  "down",
  "jump",
  "action",
  "start",
  "escape",
  "menu-select",
  "menu-back",
  "cheat-menu",
  "console",
  "peek-left",
  "peek-right",
  "peek-up",
  "peek-down",
  0
};

Controller::Controller()
{
  reset();
}

Controller::~Controller()
{}

void
Controller::reset()
{
  for(int i = 0; i < CONTROLCOUNT; ++i) {
    controls[i] = false;
    oldControls[i] = false;
  }
}

void
Controller::set_control(Control control, bool value)
{
  controls[control] = value;
}

bool
Controller::hold(Control control) const
{
  return controls[control];
}

bool
Controller::pressed(Control control) const
{
  return !oldControls[control] && controls[control];
}

bool
Controller::released(Control control) const
{
  return oldControls[control] && !controls[control];
}

void
Controller::update()
{
  for(int i = 0; i < CONTROLCOUNT; ++i)
    oldControls[i] = controls[i];
}

/* EOF */
