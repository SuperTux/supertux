//  $Id$
//
//  SuperTux
//  Copyright (C) 2004 Tobias Glaesser <tobi.web@gmx.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.

#ifndef SUPERTUX_BUTTON_H
#define SUPERTUX_BUTTON_H

#include <vector>

#include "../video/surface.h"
#include "../video/font.h"
#include "../special/timer.h"

namespace SuperTux
  {

  /// Possible states of a button.
  enum ButtonState {
    BUTTON_NONE = -1,
    BUTTON_CLICKED,
    BUTTON_PRESSED,
    BUTTON_HOVER,
    BUTTON_WHEELUP,
    BUTTON_WHEELDOWN,
    BUTTON_DEACTIVE
  };

  class ButtonPanel;

  /// Button
  /** Buttons can be placed on the screen and used like any other
      simple button known from desktop applications. */
  class Button
    {
      friend class ButtonPanel;

    public:
      /// Constructor
      Button(Surface* button_image, const std::string& ninfo,
             SDLKey nshortcut, int x, int y, int mw = -1, int mh = -1);
      Button(const std::string& imagefilename, const std::string& ninfo,
             SDLKey nshortcut, int x, int y, int mw = -1, int mh = -1);

      ~Button();

      /// Apply SDL_Event on button.
      void event(SDL_Event& event);
      /// Draw button.
      void draw(DrawingContext& context);
      /// Get button state.
      int get_state();
      /// Activate/Deactivate button.
      void set_active(bool active)
      {        active ? state = BUTTON_NONE : state = BUTTON_DEACTIVE;      };
      /// Add an icon
      /** The last added icon is the last one, which gets drawn. */
      void add_icon(const std::string& imagefile, int mw, int mh);
      /// Get position of the button on screen.
      /** Returns a SDL_Rect. */
      SDL_Rect get_pos()
      {        return rect;      }
      /// Get tag of the button
      /** Useable for button identification etc. */
      int get_tag()
      {        return tag;      }
      //  void set_drawable(Drawable* newdrawable)
      //  { drawable = newdrawable; }

      static Font* info_font;
      
    private:
      static Timer popup_timer;
      //  Drawable* drawable;
      std::vector<Surface*> icon;
      std::string info;
      SDLKey shortcut;
      SDL_Rect rect;
      bool show_info;
      ButtonState state;
      int tag;
    };

  /// Panel of buttons
  /** A ButtonPanel manages buttons inside
      its scope. It also dispatches events
      and draws the buttons it contains. */
  class ButtonPanel
    {
    public:
      /// Constructor.
      /** Expects X,Y coordinates and the width and height values
          of the ButtonPanel. */
      ButtonPanel(int x, int y, int w, int h);
      /// Constructor.
      /** SDL_Rect version of above. */
      ButtonPanel(const SDL_Rect& rect);

      ~ButtonPanel();
      /// Draw the panel and its buttons.
      void draw(DrawingContext& context);
      /// Dispatch button events.
      Button* event(SDL_Event &event);
      /// Add a button to the panel.
      /** @param tag: Can be used to identify a button. */
      void additem(Button* pbutton, int tag);
      /// Set the default size of contained buttons.
      void set_button_size(int w, int h);
      /// Manipulate a button.
      Button* manipulate_button(int i);
      /// Set if the last clicked/used item, should be drawn highlighted.
      void highlight_last(bool b);
      /// Set the last clicked/used button.
      /** Set which button is internally the last clicked/used and
          therefore drawn highlighted in case button highlighting
          is enabled for the ButtonPanel. */
      void set_last_clicked(unsigned int last);

    private:
      int bw, bh;
      bool hlast;
      bool hidden;
      SDL_Rect rect;
      std::vector<Button*> item;
      std::vector<Button*>::iterator last_clicked;
    };

} // namespace SuperTux

#endif /*SUPERTUX_BUTTON_H*/
