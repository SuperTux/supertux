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

#include "network/join_server_screen.hpp"

#include "network/client.hpp"
#include "supertux/screen_fade.hpp"
#include "supertux/screen_manager.hpp"
#include "util/log.hpp"
#include "video/compositor.hpp"
#include "video/drawing_context.hpp"
#include "video/surface.hpp"
#include "video/viewport.hpp"

#include <SDL.h>

#include <iostream>

using namespace network;

JoinServerScreen::JoinServerScreen() :
  m_client(),
  m_backbtn(Surface::from_file("/images/engine/network/back_button.png")),
  m_backbtn_hover(Surface::from_file("/images/engine/network/back_button_hover.png")),
  m_backbtn_active(Surface::from_file("/images/engine/network/back_button_active.png"))
{
}

JoinServerScreen::~JoinServerScreen()
{
}

void
JoinServerScreen::setup()
{/*
  m_client = new Client(3474, "127.0.0.1", [](Connection* c, const std::string& data) {
    log_warning << "Received data from server : {{" << data << "}}" << std::endl;
  });

  m_client->init();

  for (std::string line; std::getline(std::cin, line);) {
    if (line == "quit")
      break;
    m_client->send(line);
  }

  std::cout << "Quitting!" << std::endl;
  m_client->close();

  m_client->destroy();
*/}

void
JoinServerScreen::draw(Compositor& compositor)
{
  auto& ctx = compositor.make_context();

  ctx.color().draw_gradient(Color(0.8f, 0.9f, 1.f), Color(0.5f, 0.75f, 1.f), LAYER_GUI - 5,
                            GradientDirection::VERTICAL, ctx.get_rect());
  
  ctx.color().draw_filled_rect(ctx.get_rect().grown(-80.f), Color(0.5f, 0.7f, 0.9f), 64.f, LAYER_GUI - 3);

  ctx.color().draw_surface(m_backbtn, Vector(25, 25), LAYER_GUI - 2);
}

void
JoinServerScreen::update(float dt_sec, const Controller& controller)
{
}

void
JoinServerScreen::event(const SDL_Event& ev)
{
  if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_ESCAPE) {
    ScreenManager::current()->pop_screen();
    return;
  }
}

/* EOF */
