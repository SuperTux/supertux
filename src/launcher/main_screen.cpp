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
#include "interface/control_button.hpp"
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

LauncherMainScreen::LauncherMainScreen(bool& launch_game_on_exit) :
  m_launch_game_on_exit(launch_game_on_exit),
  m_frame(Surface::from_file("images/engine/launcher/background.png")),
  m_title(Surface::from_file("images/engine/launcher/logo.png")),
  m_copyright_text("SuperTux Launcher " PACKAGE_VERSION "\n" +
    _("Copyright") + " (c) 2020 SuperTux Devel Team\n" +
    _("This game comes with ABSOLUTELY NO WARRANTY. This is free software, and you are welcome to\n"
      "redistribute it under certain conditions; see the license file for details.\n"
      )),
  m_videosystem_name(VideoSystem::current()->get_name()),
  m_controls()
{
  m_launch_game_on_exit = false;

  auto quit = std::make_unique<ControlButton>("X");
  quit->m_on_change = new std::function<void()>([]{ ScreenManager::current()->quit(); });
  quit->set_rect(Rect(615.f, 5.f, 635.f, 25.f));
  quit->m_theme.bg_color = Color::WHITE_INVISIBLE;
  quit->m_theme.bg_hover_color = Color::WHITE_INVISIBLE;
  quit->m_theme.bg_active_color = Color::WHITE_INVISIBLE;
  quit->m_theme.bg_focus_color = Color::WHITE_INVISIBLE;
  quit->m_theme.tx_hover_color = Color(0.25f, 0.3f, 0.5f);

  UITheme theme(Color(0.8f, 0.85f, 0.9f),
                Color(0.9f, 0.925f, 0.95f),
                Color(0.7f, 0.75f, 0.8f),
                Color(0.9f, 0.925f, 0.95f),
                Color(0.3f, 0.35f, 0.4f),
                Color(0.3f, 0.35f, 0.4f),
                Color(0.3f, 0.35f, 0.4f),
                Color(0.3f, 0.35f, 0.4f),
                Resources::control_font);

  auto start = std::make_unique<ControlButton>("Launch SuperTux " PACKAGE_VERSION);
  start->m_on_change = new std::function<void()>([this]{
    m_launch_game_on_exit = true;
    ScreenManager::current()->quit();
  });
  start->set_rect(Rect(160.f, 140.f, 480.f, 180.f));
  start->m_theme = theme;

  auto version = std::make_unique<ControlButton>("Select another version");
  version->set_rect(Rect(160.f, 190.f, 480.f, 210.f));
  version->m_theme = theme;

  auto install = std::make_unique<ControlButton>("Install a new version");
  install->set_rect(Rect(160.f, 220.f, 480.f, 240.f));
  install->m_theme = theme;

  auto exit = std::make_unique<ControlButton>("Quit launcher");
  exit->m_on_change = new std::function<void()>([]{ ScreenManager::current()->quit(); });
  exit->set_rect(Rect(160.f, 300.f, 480.f, 320.f));
  exit->m_theme = theme;

  m_controls.push_back(std::move(quit));
  m_controls.push_back(std::move(start));
  m_controls.push_back(std::move(version));
  m_controls.push_back(std::move(install));
  m_controls.push_back(std::move(exit));
}

LauncherMainScreen::~LauncherMainScreen()
{
}

void
LauncherMainScreen::setup()
{
}

void
LauncherMainScreen::leave()
{
}

void
LauncherMainScreen::event(const SDL_Event& ev)
{
  for(const auto& control : m_controls)
    if (control->event(ev))
      break;
}

void
LauncherMainScreen::draw(Compositor& compositor)
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

  context.color().draw_surface_scaled(m_title,
                                      Rectf(0.f, -80.f, 640.f, 240.f),
                                      LAYER_GUI - 3);

  // UI
  Rectf box(140.f, 60.f, 500.f, 340.f);

  context.color().draw_filled_rect(box,
                                   Color(1.f, 1.f, 1.f, 1.f),
                                   10.f,
                                   LAYER_GUI - 10);

  for (float f = 2.f; f <= 20.f; f += 1.f)
    context.color().draw_filled_rect(box.grown(std::pow(f, 1.2f)).moved(Vector(0, f / 2.f)),
                                     Color(1.f, 1.f, 1.f, 0.3f / (f + 2.f)),
                                     f + 10.f,
                                     LAYER_GUI - 11);

  for(const auto& control : m_controls)
    control->draw(context);
}

void
LauncherMainScreen::update(float dt_sec, const Controller& controller)
{
  for(const auto& control : m_controls)
    control->update(dt_sec, controller);
}

/* EOF */
