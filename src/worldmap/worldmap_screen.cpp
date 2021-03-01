//  SuperTux
//  Copyright (C) 2018 Ingo Ruhnke <grumbel@gmail.com>
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

#include "worldmap/worldmap_screen.hpp"

#include "control/controller.hpp"
#include "video/compositor.hpp"
#include "worldmap/worldmap.hpp"

namespace worldmap {

WorldMapScreen::WorldMapScreen(std::unique_ptr<WorldMap> worldmap) :
  m_worldmap(std::move(worldmap))
{
}

WorldMapScreen::~WorldMapScreen()
{
}

void
WorldMapScreen::setup()
{
  m_worldmap->setup();
}

void
WorldMapScreen::leave()
{
  m_worldmap->leave();
}

void
WorldMapScreen::draw(Compositor& compositor)
{
  auto& context = compositor.make_context();
  m_worldmap->draw(context);
}

void
WorldMapScreen::update(float dt_sec, const Controller& controller)
{
  m_worldmap->process_input(controller);
  m_worldmap->update(dt_sec);
}

IntegrationStatus
WorldMapScreen::get_status() const
{
  IntegrationStatus status;
  status.m_details.push_back("In worldmap");
  status.m_details.push_back(m_worldmap->get_title());
  return status;
}

} // namespace worldmap

/* EOF */
