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

#include <string>
#include <vector>

class Controller;
class DrawingContext;

class Dialog
{
private:
  std::string m_text;
  std::vector<std::string> m_buttons;
  int m_selected_button;

public:
  Dialog();
  virtual ~Dialog();

  void set_text(const std::string& text);
  void add_button(const std::string& text);

  void process_input(const Controller& controller);
  void draw(DrawingContext& context);

  virtual void on_select(int id);

private:
  Dialog(const Dialog&) = delete;
  Dialog& operator=(const Dialog&) = delete;
};

#endif

/* EOF */
