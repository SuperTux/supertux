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

#ifndef HEADER_SUPERTUX_SUPERTUX_MAIN_HPP
#define HEADER_SUPERTUX_SUPERTUX_MAIN_HPP

#include <string>

class AddonManager;
class CommandLineArguments;
class ConfigSubsystem;
class Console;
class ConsoleBuffer;
class GameManager;
class InputManager;
class PhysfsSubsystem;
class Resources;
class Savegame;
class ScreenManager;
class SDLSubsystem;
class SoundManager;
class SpriteManager;
class SquirrelVirtualMachine;
class TileManager;
class TTFSurfaceManager;

class Main final
{
public:
  Main();

  /** We call it run() instead of main() as main collides with
      #define main SDL_main from SDL.h */
  int run(int argc, char** argv);

private:
  void init_tinygettext();
  void init_video();

  void launch_game(const CommandLineArguments& args);
  void resave(const std::string& input_filename, const std::string& output_filename);

private:
  // Using pointers allows us to initialize them whenever we want
  std::unique_ptr<PhysfsSubsystem> m_physfs_subsystem;
  std::unique_ptr<ConfigSubsystem> m_config_subsystem;
  std::unique_ptr<SDLSubsystem> m_sdl_subsystem;
  std::unique_ptr<ConsoleBuffer> m_console_buffer;
  std::unique_ptr<InputManager> m_input_manager;
  std::unique_ptr<TTFSurfaceManager> m_ttf_surface_manager;
  std::unique_ptr<SoundManager> m_sound_manager;
  std::unique_ptr<SquirrelVirtualMachine> m_squirrel_virtual_machine;
  std::unique_ptr<TileManager> m_tile_manager;
  std::unique_ptr<SpriteManager> m_sprite_manager;
  std::unique_ptr<Resources> m_resources;
  std::unique_ptr<AddonManager> m_addon_manager;
  std::unique_ptr<Console> m_console;
  std::unique_ptr<GameManager> m_game_manager;
  std::unique_ptr<ScreenManager> m_screen_manager;
  std::unique_ptr<Savegame> m_savegame;

private:
  Main(const Main&) = delete;
  Main& operator=(const Main&) = delete;
};

#endif

/* EOF */
