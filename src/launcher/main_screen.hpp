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

#ifndef HEADER_SUPERTUX_LAUNCHER_MAIN_SCREEN_HPP
#define HEADER_SUPERTUX_LAUNCHER_MAIN_SCREEN_HPP

#include "interface/control.hpp"
#include "supertux/screen.hpp"
#include "video/surface_ptr.hpp"

#include <string>

/** Screen that displays the SuperTux logo, lets players start a new
    game, etc. */
class LauncherMainScreen final : public Screen
{
public:
  LauncherMainScreen(bool& launch_game_on_exit);
  virtual ~LauncherMainScreen();

  virtual void setup() override;
  virtual void leave() override;

  virtual void event(const SDL_Event& ev) override;
  virtual void draw(Compositor& compositor) override;
  virtual void update(float dt_sec, const Controller& controller) override;

  // Needed, but irrelevant
  virtual IntegrationStatus get_status() const override { return IntegrationStatus(); }

private:
  bool& m_launch_game_on_exit;
  SurfacePtr m_frame, m_title;
  std::string m_copyright_text;
  std::string m_videosystem_name;

  std::vector<std::unique_ptr<InterfaceControl>> m_controls;

private:
  LauncherMainScreen(const LauncherMainScreen&) = delete;
  LauncherMainScreen& operator=(const LauncherMainScreen&) = delete;
};

#endif

/* EOF */
