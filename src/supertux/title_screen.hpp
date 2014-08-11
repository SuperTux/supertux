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

#ifndef HEADER_SUPERTUX_SUPERTUX_TITLE_HPP
#define HEADER_SUPERTUX_SUPERTUX_TITLE_HPP

#include "supertux/game_session.hpp"

class AddonMenu;
class CodeController;
class ContribMenu;
class ContribWorldMenu;
class Menu;
class PlayerStatus;
class World;

/**
 * Screen that displays the SuperTux logo, lets players start a new game, etc.
 */
class TitleScreen : public Screen
{
public:
  TitleScreen(PlayerStatus* player_status);
  virtual ~TitleScreen();

  virtual void setup();
  virtual void leave();

  virtual void draw(DrawingContext& context);

  virtual void update(float elapsed_time);

private:
  void make_tux_jump();
  
private:
  SurfacePtr frame;
  std::unique_ptr<CodeController> controller;
  std::unique_ptr<GameSession> titlesession;
  std::string copyright_text;

private:
  TitleScreen(const TitleScreen&);
  TitleScreen& operator=(const TitleScreen&);
};

#endif

/* EOF */
