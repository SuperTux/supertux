//  SuperTux
//  Copyright (C) 2020 A. Semphris <semphris@protonmail.com>
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

#include "config.h"

#include "sdk/integration.hpp"

#ifdef DISCORD_ENABLED
#include "sdk/discord.hpp"
#endif

Integration* Integration::sdks[] = {
#ifdef DISCORD_ENABLED
  DiscordIntegration::getSingleton()
#endif
};

IntegrationStatus Integration::m_status = MAIN_MENU;

void
Integration::init_all()
{
  for (Integration* sdk : sdks)
    sdk->init();
}

void
Integration::update_all()
{
  for (Integration* sdk : sdks)
    sdk->update();
}

void
Integration::close_all()
{
  for (Integration* sdk : sdks)
    sdk->close();
}

void
Integration::set_status(IntegrationStatus status)
{
  m_status = status;

  for (Integration* sdk : sdks)
    sdk->update_status(status);
}

void
Integration::set_worldmap(const char* worldmap)
{
  for (Integration* sdk : sdks)
    sdk->update_worldmap(worldmap);
}

void
Integration::set_level(const char* level)
{
  for (Integration* sdk : sdks)
    sdk->update_level(level);
}

IntegrationStatus
Integration::get_status()
{
  return m_status;
}
