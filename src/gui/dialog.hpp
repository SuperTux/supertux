//  SuperTux
//  Copyright (C) 2014 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_SUPERTUX_GUI_DIALOG_HPP
#define HEADER_SUPERTUX_GUI_DIALOG_HPP

#include <SDL.h>
#include <functional>
#include <string>
#include <vector>

#include "math/sizef.hpp"

class Controller;
class DrawingContext;

class Dialog
{
private:
  struct Button
  {
    std::string text;
    std::function<void ()> callback;
  };

  std::string m_text;
  std::vector<Button> m_buttons;
  int m_selected_button;
  int m_cancel_button;
  int m_passive;

  Sizef m_text_size;

public:
  Dialog(bool passive = false);
  virtual ~Dialog();

  void set_text(const std::string& text);

  void add_button(const std::string& text, const std::function<void ()>& callback = {});

  /** The default gets focused when the dialog is first shown */
  void add_default_button(const std::string& text, const std::function<void ()>& callback = {});

  /** The cancel button can not only be activated by selecting it, but
      via the MENU_BACK button */
  void add_cancel_button(const std::string& text, const std::function<void ()>& callback = {});

  void clear_buttons();

  void event(const SDL_Event& event);
  void process_input(const Controller& controller);
  void draw(DrawingContext& context);
  virtual void update() {}
  bool is_passive() const
  {
    return m_passive;
  }

private:
  void on_button_click(int button) const;
  int get_button_at(const Vector& pos) const;

private:
  Dialog(const Dialog&) = delete;
  Dialog& operator=(const Dialog&) = delete;
};

#endif

/* EOF */
