//  SuperTux
//  Copyright (C) 2015 Hume2 <teratux.mail@gmail.com>
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

#include <vector>

#include "editor/input_gui.hpp"

#include "editor/editor.hpp"
#include "editor/object_group.hpp"
#include "editor/object_input.hpp"
#include "gui/menu_manager.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/menu/editor_tilegroup_menu.hpp"
#include "supertux/colorscheme.hpp"
#include "supertux/globals.hpp"
#include "supertux/level.hpp"
#include "supertux/resources.hpp"
#include "supertux/tile.hpp"
#include "supertux/tile_manager.hpp"
#include "supertux/tile_set.hpp"
#include "util/gettext.hpp"
#include "video/drawing_context.hpp"
#include "video/font.hpp"
#include "video/renderer.hpp"
#include "video/video_system.hpp"

EditorInputGui::EditorInputGui() :
  tile(0),
  object(),
  input_type(IP_NONE),
  active_tilegroup(),
  active_objectgroup(-1),
  object_input(),
  hovered_item(HI_NONE),
  hovered_tile(-1),
  tile_scrolling(TS_NONE),
  starting_tile(0),
  Xpos(512)
{
  std::unique_ptr<ObjectInput> oi( new ObjectInput() );
  object_input = move(oi);
}

EditorInputGui::~EditorInputGui() {

}

void
EditorInputGui::draw(DrawingContext& context) {
  //SCREEN_WIDTH SCREEN_HEIGHT
  context.draw_filled_rect(Rectf(Vector(Xpos, 0), Vector(SCREEN_WIDTH, SCREEN_HEIGHT)),
                           Color(0.9f, 0.9f, 1.0f, 0.6f),
                           0.0f,
                           LAYER_GUI-10);

  switch (hovered_item) {
    case HI_TILEGROUP:
      context.draw_filled_rect(Rectf(Vector(Xpos, 0), Vector(SCREEN_WIDTH, 22)),
                               Color(0.9f, 0.9f, 1.0f, 0.6f),
                               0.0f,
                               LAYER_GUI-5);
      break;
    case HI_OBJECTS:
      context.draw_filled_rect(Rectf(Vector(Xpos, 22), Vector(SCREEN_WIDTH, 44)),
                               Color(0.9f, 0.9f, 1.0f, 0.6f),
                               0.0f,
                               LAYER_GUI-5);
      break;
    case HI_TILE: {
      Vector coords = get_tile_coords(hovered_tile);
      context.draw_filled_rect(Rectf(coords, coords + Vector(32, 32)),
                               Color(0.9f, 0.9f, 1.0f, 0.6f),
                               0.0f,
                               LAYER_GUI-5);
    } break;
    default: break;
  }

  context.draw_text(Resources::normal_font, _("Tilegroups"),
                    Vector(SCREEN_WIDTH, 0),
                    ALIGN_RIGHT, LAYER_GUI, ColorScheme::Menu::default_color);
  context.draw_text(Resources::normal_font, _("Objects"),
                    Vector(SCREEN_WIDTH, 24),
                    ALIGN_RIGHT, LAYER_GUI, ColorScheme::Menu::default_color);

  draw_tilegroup(context);
  draw_objectgroup(context);
}

void
EditorInputGui::draw_tilegroup(DrawingContext& context) {
  if (input_type == IP_TILE) {
    int pos = -1;
    for(auto i = active_tilegroup.begin(); i != active_tilegroup.end(); ++i) {
      pos++;
      if (pos < starting_tile) {
        continue;
      }
      int* tile_ID = &(*i);
      if ((*tile_ID) == 0) {
        continue;
      }
      const Tile* tg_tile = Editor::current()->level->tileset->get(*tile_ID);
      tg_tile->draw(context, get_tile_coords(pos - starting_tile), LAYER_GUI-9);
    }
  }
}

void
EditorInputGui::draw_objectgroup(DrawingContext& context) {
  if (input_type == IP_OBJECT) {
    int pos = -1;
    for(auto i = object_input->groups[active_objectgroup].icons.begin();
        i != object_input->groups[active_objectgroup].icons.end(); ++i) {
      pos++;
      if (pos < starting_tile) {
        continue;
      }
      ObjectIcon* oi = &(*i);
      oi->draw(context, get_tile_coords(pos - starting_tile));
    }
  }
}

void
EditorInputGui::update(float elapsed_time) {
  switch (tile_scrolling) {
    case TS_UP: if (starting_tile > 0) {
        starting_tile -= 4;
      } break;
    case TS_DOWN: {
      int size;
      if (input_type == IP_OBJECT){
        size = object_input->groups[active_objectgroup].icons.size();
      }else{
        size = active_tilegroup.size();
      }
      if (starting_tile < size-5) {
        starting_tile += 4;
      }
    }
    default: break;
  }
}

bool
EditorInputGui::event(SDL_Event& ev) {
  switch (ev.type) {
    case SDL_MOUSEBUTTONDOWN:
    {
      if(ev.button.button == SDL_BUTTON_LEFT)
      {
        switch (hovered_item) {
          case HI_TILEGROUP:
            Editor::current()->disable_keyboard();
            MenuManager::instance().push_menu(MenuStorage::EDITOR_TILEGROUP_MENU);
            break;
          case HI_OBJECTS:
            Editor::current()->disable_keyboard();
            MenuManager::instance().push_menu(MenuStorage::EDITOR_OBJECTGROUP_MENU);
            break;
          default:
            return false;
            break;
        }
      }
    } break;

    case SDL_MOUSEMOTION:
    {
      Vector mouse_pos = VideoSystem::current()->get_renderer().to_logical(ev.motion.x, ev.motion.y);
      float x = mouse_pos.x - Xpos;
      float y = mouse_pos.y - Ypos;
      if (x < 0) {
        hovered_item = HI_NONE;
        tile_scrolling = TS_NONE;
        return false;
        break;
      }
      if (y < 0) {
        if (y < -22) {
          hovered_item = HI_TILEGROUP;
        }else{
          hovered_item = HI_OBJECTS;
        }
        tile_scrolling = TS_NONE;
        break;
      }else{
        hovered_item = HI_TILE;
        hovered_tile = get_tile_pos(mouse_pos);
      }
      if (y < 16) {
        tile_scrolling = TS_UP;
      }else if (y > SCREEN_HEIGHT - 16 - Ypos) {
        tile_scrolling = TS_DOWN;
      }else{
        tile_scrolling = TS_NONE;
      }
    }
    break;
    default:
      return false;
      break;
  }
  return true;
}

void
EditorInputGui::resize() {
  Xpos = SCREEN_WIDTH - 128;
}

void
EditorInputGui::setup() {
  resize();
  tile = 0;
}

Vector
EditorInputGui::get_tile_coords(const int pos){
  int x = pos%4;
  int y = pos/4;
  return Vector( x * 32 + Xpos, y * 32 + Ypos);
}

int
EditorInputGui::get_tile_pos(const Vector coords){
  int x = (coords.x - Xpos) / 32;
  int y = (coords.y - Ypos) / 32;
  return y*4 + x;
}

/* EOF */
