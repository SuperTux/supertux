//
// C Interface: button
//
// Description:
//
//
// Author: Tobias Glaesser <tobi.web@gmx.de>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef SUPERTUX_BUTTON_H
#define SUPERTUX_BUTTON_H

#include <vector>
#include "texture.h"

enum ButtonState {
  BUTTON_NONE = -1,
  BUTTON_CLICKED,
  BUTTON_PRESSED,
  BUTTON_HOVER
};

class ButtonPanel;

class Button
  {
    friend class ButtonPanel;

  public:
    Button(std::string icon_file, std::string info, SDLKey shortcut, int x, int y, int mw = -1, int h = -1);
    ~Button();
    void event(SDL_Event& event);
    void draw();
    int get_state();
    void change_icon(std::string icon_file, int mw, int mh);
    int get_tag()
    {
      return tag;
    }

  private:
    static timer_type popup_timer;
    texture_type icon;
    texture_type* bkgd;
    std::string info;
    SDLKey shortcut;
    SDL_Rect rect;
    bool show_info;
    ButtonState state;
    int tag;
  };

class ButtonPanel
  {
  public:
    ButtonPanel(int x, int y, int w, int h);
    ~ButtonPanel();
    void draw();
    Button* event(SDL_Event &event);
    void additem(Button* pbutton, int tag);
    Button* button_panel_event(SDL_Event& event);
    void set_button_size(int w, int h) { bw = w; bh = h; }

  private:
    int bw, bh;
    bool hidden;
    SDL_Rect rect;
    std::vector<Button*> item;
  };

#endif /*SUPERTUX_BUTTON_H*/
