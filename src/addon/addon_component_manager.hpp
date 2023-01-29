//  SuperTux
//  Copyright (C) 2022 Vankata453
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

#ifndef HEADER_SUPERTUX_ADDON_ADDON_COMPONENT_MANAGER_HPP
#define HEADER_SUPERTUX_ADDON_ADDON_COMPONENT_MANAGER_HPP

/** Represents a class, which helps manage additional Add-on components */
class AddonComponentManager
{
public:
  AddonComponentManager() {}
  virtual ~AddonComponentManager() {}

  virtual void update() = 0;

private:
  AddonComponentManager(const AddonComponentManager&) = delete;
  AddonComponentManager& operator=(const AddonComponentManager&) = delete;
};

#endif

/* EOF */
