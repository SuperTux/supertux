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

#include "launcher/main_screen.hpp"

#include <version.h>

#include "gui/menu_manager.hpp"
#include "launcher/dialog_main.hpp"
#include "object/camera.hpp"
#include "object/music_object.hpp"
#include "object/player.hpp"
#include "supertux/fadetoblack.hpp"
#include "supertux/game_session.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/resources.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/sector.hpp"
#include "video/compositor.hpp"
#include "video/drawing_context.hpp"
#include "video/surface.hpp"
#include "video/video_system.hpp"

MainScreen::MainScreen(char* arg0) :
  m_frame(Surface::from_file("images/engine/launcher/background.png")),
  m_controller(new CodeController()),
  m_copyright_text("SuperTux Launcher " PACKAGE_VERSION "\n" +
    _("Copyright") + " (c) 2020 SuperTux Devel Team\n" +
    _("This game comes with ABSOLUTELY NO WARRANTY. This is free software, and you are welcome to\n"
      "redistribute it under certain conditions; see the license file for details.\n"
      )),
  m_videosystem_name(VideoSystem::current()->get_name()),
  m_arg0(arg0)
{
}

MainScreen::~MainScreen()
{
}

void
MainScreen::setup()
{
  MenuManager::instance().set_menu(std::make_unique<LauncherMainMenu>(m_arg0));
}

void
MainScreen::leave()
{
  MenuManager::instance().clear_menu_stack();
}

void
MainScreen::draw(Compositor& compositor)
{
  auto& context = compositor.make_context();

  float delta_x = 640.f - static_cast<float>(context.get_width()) / 2.f;
  float delta_y = 360.f - static_cast<float>(context.get_height()) / 2.f;
  context.color().draw_surface_scaled(m_frame,
                                      Rectf(
                                        -delta_x,
                                        -delta_y,
                                        -delta_x + 1280.f,
                                        -delta_y + 720.f
                                      ),
                                      LAYER_BACKGROUND1);
  context.color().draw_text(Resources::small_font,
                            m_copyright_text,
                            Vector(5.0f, static_cast<float>(context.get_height()) - 50.0f),
                            ALIGN_LEFT, LAYER_FOREGROUND1);

  context.color().draw_text(Resources::small_font,
                            m_videosystem_name,
                            Vector(static_cast<float>(context.get_width()) - 5.0f,
                                   static_cast<float>(context.get_height()) - 14.0f),
                            ALIGN_RIGHT, LAYER_FOREGROUND1);
}

void
MainScreen::update(float dt_sec, const Controller& controller)
{
  // reopen menu if user closed it (so that the app doesn't close when user
  // accidently hit ESC)
  if (!MenuManager::instance().is_active())
  {
    MenuManager::instance().set_menu(std::make_unique<LauncherMainMenu>(m_arg0));
  }
}

/* EOF */
