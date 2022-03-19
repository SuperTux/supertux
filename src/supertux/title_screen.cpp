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

#include "supertux/title_screen.hpp"

#include <version.h>

#include "gui/menu_manager.hpp"
#include "object/camera.hpp"
#include "object/music_object.hpp"
#include "object/player.hpp"
#include "sdk/integration.hpp"
#include "supertux/fadetoblack.hpp"
#include "supertux/game_session.hpp"
#include "supertux/level.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/resources.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/sector.hpp"
#include "video/compositor.hpp"
#include "video/drawing_context.hpp"
#include "video/surface.hpp"
#include "video/video_system.hpp"

TitleScreen::TitleScreen(Savegame& savegame) :
  m_frame(Surface::from_file("images/engine/menu/frame.png")),
  m_controller(new CodeController()),
  m_titlesession(new GameSession("levels/misc/menu.stl", savegame)),
  m_copyright_text("SuperTux " PACKAGE_VERSION "\n" +
    _("Copyright") + " (c) 2003-2022 SuperTux Devel Team\n" +
    _("This game comes with ABSOLUTELY NO WARRANTY. This is free software, and you are welcome to\n"
      "redistribute it under certain conditions; see the license file for details.\n"
      )),
  m_videosystem_name(VideoSystem::current()->get_name())
{
  Player& player = m_titlesession->get_current_sector().get_player();
  player.set_controller(m_controller.get());
  player.set_speedlimit(230); //MAX_WALK_XM
}

void
TitleScreen::make_tux_jump()
{
  static bool jumpWasReleased = false;
  Sector& sector  = m_titlesession->get_current_sector();
  Player& tux = sector.get_player();

  m_controller->update();
  m_controller->press(Control::RIGHT);

  // Check if we should press the jump button
  Rectf lookahead = tux.get_bbox();
  lookahead.set_right(lookahead.get_right() + 96);
  lookahead.set_bottom(lookahead.get_bottom() - 2);
  bool pathBlocked = !sector.is_free_of_statics(lookahead);
  if ((pathBlocked && jumpWasReleased) || tux.m_fall_mode == Player::FallMode::JUMPING) {
    m_controller->press(Control::JUMP);
    jumpWasReleased = false;
  } else {
    jumpWasReleased = true;
  }

  // Wrap around at the end of the level back to the beginning
  if (sector.get_width() - 320 < tux.get_pos().x) {
    sector.activate("main");
    sector.get_camera().reset(tux.get_pos());
  }
}

TitleScreen::~TitleScreen()
{
}

void
TitleScreen::setup()
{
  Sector& sector = m_titlesession->get_current_sector();
  if (Sector::current() != &sector) {
    auto& music = sector.get_singleton_by_type<MusicObject>();
    music.play_music(LEVEL_MUSIC);
    // sector.activate(Vector) expects position calculated for big tux, but tux
    // might be small on the title screen
    sector.activate(sector.get_player().get_pos() - Vector(0.f, sector.get_player().is_big() ? 0.f : 32.f));
  }

  MenuManager::instance().set_menu(MenuStorage::MAIN_MENU);
  ScreenManager::current()->set_screen_fade(std::make_unique<FadeToBlack>(FadeToBlack::FADEIN, 0.25f));
}

void
TitleScreen::leave()
{
  Sector& sector = m_titlesession->get_current_sector();
  sector.deactivate();
  MenuManager::instance().clear_menu_stack();
}

void
TitleScreen::draw(Compositor& compositor)
{
  auto& context = compositor.make_context();

  Sector& sector  = m_titlesession->get_current_sector();
  sector.draw(context);

  context.color().draw_surface_scaled(m_frame,
                                      Rectf(0, 0, static_cast<float>(context.get_width()), static_cast<float>(context.get_height())),
                                      LAYER_FOREGROUND1);

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
TitleScreen::update(float dt_sec, const Controller& controller)
{
  ScreenManager::current()->set_speed(0.6f);
  Sector& sector  = m_titlesession->get_current_sector();
  sector.update(dt_sec);

  BIND_SECTOR(sector);
  make_tux_jump();

  // reopen menu if user closed it (so that the app doesn't close when user
  // accidently hit ESC)
  if (!MenuManager::instance().is_active() && !ScreenManager::current()->has_pending_fadeout())
  {
    MenuManager::instance().set_menu(MenuStorage::MAIN_MENU);
  }
}

IntegrationStatus
TitleScreen::get_status() const
{
  IntegrationStatus status;
  status.m_details.push_back("In main menu");
  return status;
}

/* EOF */
