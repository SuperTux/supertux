//  SuperTux
//  Copyright (C) 2026 Tobias Markus <tobbi.bugs@googlemail.com>
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

#include "math/rectf.hpp"
#include "math/vector.hpp"
#include "supertux/globals.hpp"
#include "video/drawing_context.hpp"
#include "video/video_system.hpp"
#include "video/viewport.hpp"

class Color;
class GameObject;
class InterfaceControl;

/**
 * Class describing the editor properties panel UI component
 */
class EditorPropertiesPanel
{
public:
  /**
   * Constructor
   */
  EditorPropertiesPanel();

  /**
   * Returns the area that the properties panel covers
   */
  Rectf get_area() const;

  /**
   * Returns the background color of the component
   */
  Color get_background_color() const;

  /**
   * Returns the layer of this component
   */
  int get_layer() const;

  /**
   * Method called on update
   * @param dt_sec elapsed time since last call
   */
  void update(float dt_sec);

  /**
   * Method called on draw
   * @param context DrawingContext instance
   */
  void draw(DrawingContext& context);

  /**
   * Returns `true` when the properties panel has focus
   * @return `true`, when the properties panel has focus, otherwise `false`
   */
  bool has_focus() const;

  /**
   * Returns `true` when the properties panel should handle the current mouse event
   * @param ev Instance of `SDL_Event` class describing the current event
   * @param mouse_pos Vector instance describing the current mouse position
   * @return `true`, when the properties panel has mouse focus, otherwise `false`
   */
  bool has_mouse_focus(const SDL_Event& ev, const Vector& mouse_pos) const;

  /**
   * Handles the current SDL_Event in case it's handled by this component
   * @param ev Instance of `SDL_Event` class describing the current event
   * @return `true` when the current event was handled by this component, otherwise `false`
   */
  bool event(const SDL_Event &ev);

  /**
   * Loads the properties for the specified `GameObject` instance
   * @param object `GameObject` instance to load the properties for.
   */
  void load_object_properties(GameObject *object);

  /**
   * Returns `true`, when the properties panel is currently visible, otherwise `false`
   */
  bool is_visible() const;

  /**
   * Checks if `pos` is inside the properties panel's area
   * @param pos Position to check
   * @return true when the position is inside the properties panel's area, otherwise `false`.
   */
  bool contains_position(const Vector &pos) const;

  /**
   * Removes all controls from the properties panel
   */
  void clear() { m_controls.clear(); }

private:
  /**
   * Adds a new control to the properties panel
   * @param name Name of the control to add
   * @param new_control `InterfaceControl` instance of the new control
   * @param description Optional description string containing more information about the current control
   */
  void add_control(const std::string& name, std::unique_ptr<InterfaceControl> new_control, const std::string& description = "");

private:

  /**
   * List of controls for the currently selected `GameObject`
   */
  std::vector<std::unique_ptr<InterfaceControl>> m_controls;
};
