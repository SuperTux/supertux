//  SuperTux
//  Copyright (C) 2025 Tobias Markus <tobbi.bugs@gmail.com>
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

#pragma once

#include "editor/widget.hpp"

#include <array>
#include <memory>

#include "math/rectf.hpp"
#include "math/vector.hpp"
#include "video/surface_ptr.hpp"

class Editor;
class EditorToolbarButtonWidget;

/** The toolbar is at the top of the screen and offers easy access to the most commonly used functions of the editor. */
class EditorToolbarWidget final : public Widget
{

public:
  EditorToolbarWidget(Editor& editor);

  virtual void draw(DrawingContext& context) override;
  virtual void update(float dt_sec) override;
  virtual bool event(const SDL_Event& ev) override;

  void toggle_tile_object_mode();

  void set_undo_disabled(bool state);
  void set_redo_disabled(bool state);

private:
  Editor& m_editor;
  EditorToolbarButtonWidget* m_undo_widget;
  EditorToolbarButtonWidget* m_redo_widget;
  std::vector<std::unique_ptr<EditorToolbarButtonWidget> > m_widgets;

  float m_widgets_width;
  float m_widgets_width_offset;

private:
  EditorToolbarWidget(const EditorToolbarWidget&) = delete;
  EditorToolbarWidget& operator=(const EditorToolbarWidget&) = delete;
};
