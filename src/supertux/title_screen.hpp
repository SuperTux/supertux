//  SuperTux
//  Copyright (C) 2004 Tobias Glaesser <tobi.web@gmx.de>
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

#ifndef HEADER_SUPERTUX_SUPERTUX_TITLE_SCREEN_HPP
#define HEADER_SUPERTUX_SUPERTUX_TITLE_SCREEN_HPP

#include "supertux/screen.hpp"
#include "video/surface_ptr.hpp"

#include <string>

class CodeController;
class GameSession;
class Savegame;

/** Screen that displays the SuperTux logo, lets players start a new
    game, etc. */
class TitleScreen final : public Screen
{
public:
  TitleScreen(Savegame& savegame);
  ~TitleScreen() override;

  virtual void setup() override;
  virtual void leave() override;

  virtual void draw(Compositor& compositor) override;
  virtual void update(float dt_sec, const Controller& controller) override;

  virtual IntegrationStatus get_status() const override;

private:
  void make_tux_jump();

private:
  SurfacePtr m_frame;
  std::unique_ptr<CodeController> m_controller;
  std::unique_ptr<GameSession> m_titlesession;
  std::string m_copyright_text;
  std::string m_videosystem_name;

private:
  TitleScreen(const TitleScreen&) = delete;
  TitleScreen& operator=(const TitleScreen&) = delete;
};

#endif

/* EOF */
