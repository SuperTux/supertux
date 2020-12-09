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

#include "supertux/screen.hpp"
#include "video/surface_ptr.hpp"

#include <string>

class CodeController;
class GameSession;
class Savegame;

/** Screen that displays the SuperTux logo, lets players start a new
    game, etc. */
class MainScreen final : public Screen
{
public:
  MainScreen(char* arg0);
  virtual ~MainScreen();

  virtual void setup() override;
  virtual void leave() override;

  virtual void draw(Compositor& compositor) override;
  virtual void update(float dt_sec, const Controller& controller) override;

  // Needed, but irrelevant
  virtual IntegrationStatus get_status() const override { return IntegrationStatus(); }

private:
  SurfacePtr m_frame;
  std::unique_ptr<CodeController> m_controller;
  std::string m_copyright_text;
  std::string m_videosystem_name;
  char* m_arg0;

private:
  MainScreen(const MainScreen&) = delete;
  MainScreen& operator=(const MainScreen&) = delete;
};

#endif

/* EOF */
