//  SuperTux
//  Copyright (C) 2004-2018 Ingo Ruhnke <grumbel@gmail.com>
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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

#ifndef HEADER_SUPERTUX_WORLDMAP_WORLDMAP_SCREEN_HPP
#define HEADER_SUPERTUX_WORLDMAP_WORLDMAP_SCREEN_HPP

#include <memory>

#include "supertux/screen.hpp"
#include "util/currenton.hpp"

namespace worldmap {

class WorldMap;

class WorldMapScreen final : public Screen,
                       public Currenton<WorldMapScreen>
{
public:
  WorldMapScreen(std::unique_ptr<WorldMap> worldmap);
  ~WorldMapScreen() override;

  virtual void setup() override;
  virtual void leave() override;

  virtual void draw(Compositor& compositor) override;
  virtual void update(float dt_sec, const Controller& controller) override;
  virtual IntegrationStatus get_status() const override;

private:
  std::unique_ptr<WorldMap> m_worldmap;

private:
  WorldMapScreen(const WorldMapScreen&) = delete;
  WorldMapScreen& operator=(const WorldMapScreen&) = delete;
};

} // namespace worldmap

#endif

/* EOF */
