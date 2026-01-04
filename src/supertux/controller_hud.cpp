//  SuperTux
//  Copyright (C) 2018 Ingo Ruhnke <grumbel@gmail.com>
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

#include "supertux/controller_hud.hpp"

#include "control/input_manager.hpp"
#include "editor/editor.hpp"
#include "math/vector.hpp"
#include "supertux/savegame.hpp"
#include "supertux/game_session.hpp"
#include "worldmap/worldmap.hpp"
#include "video/drawing_context.hpp"

#define CONTROL_INT(x) (static_cast<std::underlying_type_t<Control>>(Control::x))

ControllerHUD::ControllerHUD() :
  m_controls()
{
  update_pos();

}

void
ControllerHUD::update_pos()
{
  Vector pos(128, 64);
  const Sizef btn_size(16, 16);

  // Technically, the worldmap is loaded while playing the game...
  if (worldmap::WorldMap::current() ||
      (Editor::current() && !Editor::current()->is_active()))
  {
    pos.x += 64;
  }

  const Vector dpad_pos = pos + Vector(-64.0f, 0.0f);
  m_controls[CONTROL_INT(LEFT)] = Rectf::from_center(dpad_pos + Vector(-16.0f, 0.0f), btn_size);
  m_controls[CONTROL_INT(RIGHT)] = Rectf::from_center(dpad_pos + Vector(16.0f, 0.0f), btn_size);
  m_controls[CONTROL_INT(UP)] = Rectf::from_center(dpad_pos + Vector(0.0f, -16.0f), btn_size);
  m_controls[CONTROL_INT(DOWN)] = Rectf::from_center(dpad_pos + Vector(0.0f, 16.0f), btn_size);

  const Vector peek_pos = pos + Vector(0.0f, -24.0f);
  m_controls[CONTROL_INT(PEEK_LEFT)] = Rectf::from_center(peek_pos + Vector(-16.0f, 0.0f), btn_size);
  m_controls[CONTROL_INT(PEEK_RIGHT)] = Rectf::from_center(peek_pos + Vector(16.0f, 0.0f), btn_size);
  m_controls[CONTROL_INT(PEEK_UP)] = Rectf::from_center(peek_pos + Vector(0.0f, -16.0f), btn_size);
  m_controls[CONTROL_INT(PEEK_DOWN)] = Rectf::from_center(peek_pos + Vector(0.0f, 16.0f), btn_size);

  const Vector btn_pos = pos + Vector(64.0f, -8.0f);
  m_controls[CONTROL_INT(ACTION)] = Rectf::from_center(btn_pos + Vector(-20.0f, 0.0f), btn_size);
  m_controls[CONTROL_INT(JUMP)] = Rectf::from_center(btn_pos + Vector(0.0f, 20.0f), btn_size);

  m_controls[CONTROL_INT(ITEM)] = Rectf::from_center(pos + Vector(-24.f, 24.0f), Sizef(16, 8));
  m_controls[CONTROL_INT(ESCAPE)] = Rectf::from_center(pos + Vector(0.0f, 24.0f), Sizef(16, 8));
  m_controls[CONTROL_INT(START)] = Rectf::from_center(pos + Vector(24.0f, 24.0f), Sizef(16, 8));
}

void
ControllerHUD::draw(DrawingContext& context)
{
  if (Editor::current() && Editor::current()->is_active())
    return;
  Canvas& canvas = context.color();
  Controller& controller = InputManager::current()->get_controller();

  update_pos();

  for (int i = 0; i < CONTROL_INT(CONTROLCOUNT); ++i)
  {
    if (controller.pressed(static_cast<Control>(i))) {
      canvas.draw_filled_rect(m_controls[i], Color::WHITE, LAYER_HUD);
    } else if (controller.released(static_cast<Control>(i))) {
      canvas.draw_filled_rect(m_controls[i], Color::BLACK, LAYER_HUD);
    } else if (controller.hold(static_cast<Control>(i))) {
      canvas.draw_filled_rect(m_controls[i], Color::RED, LAYER_HUD);
    } else {
      canvas.draw_filled_rect(m_controls[i], Color(0.5f, 0.5f, 0.5f), LAYER_HUD);
    }
  }
}
