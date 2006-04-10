//  $Id$
// 
//  SuperTux
//  Copyright (C) 2000 Bill Kendrick <bill@newbreedsoftware.com>
//  Copyright (C) 2004 Tobias Glaesser <tobi.web@gmx.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.
#ifndef SUPERTUX_TITLE_H
#define SUPERTUX_TITLE_H

#include <memory>
#include <vector>
#include "screen.hpp"
#include "game_session.hpp"

class Menu;
class World;
class CodeController;

class TitleScreen : public Screen
{
public:
  TitleScreen();
  virtual ~TitleScreen();

  virtual void setup();
  virtual void leave();

  virtual void draw(DrawingContext& context);

  virtual void update(float elapsed_time);

private:
  std::string get_slotinfo(int slot);
  std::string get_level_name(const std::string& levelfile);
  bool process_load_game_menu();
  void make_tux_jump();
  void update_load_game_menu();
  void generate_contrib_menu();
  void check_levels_contrib_menu();
  void check_contrib_world_menu();
  void free_contrib_menu();

  std::auto_ptr<Menu> load_game_menu;
  std::auto_ptr<Menu> contrib_menu;
  std::auto_ptr<Menu> contrib_world_menu;
  std::vector<World*> contrib_worlds;
  int current_world;
  World* current_contrib_world;
  
  std::auto_ptr<CodeController> controller;
  std::auto_ptr<GameSession> titlesession;
};

#endif

