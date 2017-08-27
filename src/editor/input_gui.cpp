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
#include "editor/tile_selection.hpp"
#include "editor/tool_icon.hpp"
#include "gui/menu_manager.hpp"
#include "gui/mousecursor.hpp"
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
#include "util/log.hpp"
#include "video/drawing_context.hpp"
#include "video/font.hpp"
#include "video/renderer.hpp"
#include "video/video_system.hpp"

EditorInputGui::EditorInputGui() :
  tiles(new TileSelection()),
  object(),
  input_type(IP_NONE),
  active_tilegroup(),
  active_objectgroup(-1),
  object_input(),
  rubber(       new ToolIcon("images/engine/editor/rubber.png")),
  select_mode(  new ToolIcon("images/engine/editor/select-mode0.png")),
  move_mode(    new ToolIcon("images/engine/editor/move-mode0.png")),
  settings_mode(new ToolIcon("images/engine/editor/settings-mode0.png")),
  hovered_item(HI_NONE),
  hovered_tile(-1),
  tile_scrolling(TS_NONE),
  using_scroll_wheel(false),
  wheel_scroll_amount(0),
  starting_tile(0),
  dragging(false),
  drag_start(0, 0),
  Xpos(512)
{
  std::unique_ptr<ObjectInput> oi( new ObjectInput() );
  object_input = move(oi);

  select_mode->push_mode  ("images/engine/editor/select-mode1.png");
  select_mode->push_mode  ("images/engine/editor/select-mode2.png");
  move_mode->push_mode    ("images/engine/editor/move-mode1.png");
  //settings_mode->push_mode("images/engine/editor/settings-mode1.png");
}

EditorInputGui::~EditorInputGui() {

}

void
EditorInputGui::draw(DrawingContext& context) {
  //SCREEN_WIDTH SCREEN_HEIGHT
  context.draw_filled_rect(Rectf(Vector(Xpos, 0), Vector(SCREEN_WIDTH, SCREEN_HEIGHT)),
                           Color(0.9f, 0.9f, 1.0f, 0.6f),
                           0.0f, LAYER_GUI-10);
  if (dragging) {
    context.draw_filled_rect(selection_draw_rect(), Color(0.2f, 0.4f, 1.0f, 0.6f),
                             0.0f, LAYER_GUI+1);
  }

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
    case HI_TOOL: {
      Vector coords = get_tool_coords(hovered_tile);
      context.draw_filled_rect(Rectf(coords, coords + Vector(32, 16)),
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

  rubber->draw(context);
  select_mode->draw(context);
  move_mode->draw(context);
  settings_mode->draw(context);

  draw_tilegroup(context);
  draw_objectgroup(context);
}

void
EditorInputGui::draw_tilegroup(DrawingContext& context) {
  if (input_type == IP_TILE) {
    int pos = -1;
    for(auto& tile_ID : active_tilegroup) {
      pos++;
      if (pos < starting_tile) {
        continue;
      }
      Editor::current()->tileset->draw_tile(context, tile_ID, get_tile_coords(pos - starting_tile), LAYER_GUI-9);
      /*if (tile_ID == 0) {
        continue;
      }
      const Tile* tg_tile = Editor::current()->tileset->get(tile_ID);
      tg_tile->draw(context, get_tile_coords(pos - starting_tile), LAYER_GUI-9);*/
    }
  }
}

void
EditorInputGui::draw_objectgroup(DrawingContext& context) {
  if (input_type == IP_OBJECT) {
    int pos = -1;
    for(auto& icon : object_input->groups[active_objectgroup].icons) {
      pos++;
      if (pos < starting_tile) {
        continue;
      }
      icon.draw(context, get_tile_coords(pos - starting_tile));
    }
  }
}

void
EditorInputGui::update(float elapsed_time) {
  switch (tile_scrolling) {
    case TS_UP: 
    {
        if (starting_tile > 0) 
        {
          if(using_scroll_wheel)
          {
            starting_tile -= 4 * wheel_scroll_amount;
            if (starting_tile < 0)
            {
              starting_tile = 0;
            }
            tile_scrolling = TS_NONE;
          }
          else
          {
            starting_tile -= 4;
          }
        }
    }
      break;
    case TS_DOWN: {
      int size;
      if (input_type == IP_OBJECT){
        size = object_input->groups[active_objectgroup].icons.size();
      } else {
        size = active_tilegroup.size();
      }
      if (starting_tile < size-5) {
        if(using_scroll_wheel)
        {
          starting_tile -= 4 * wheel_scroll_amount;
          if (starting_tile > size - 4)
          {
            starting_tile = size - 4;
          }
          tile_scrolling = TS_NONE;
        }
        else
        {
          starting_tile += 4;
        }
      }
    }
    default: break;
  }
}

Rectf
EditorInputGui::normalize_selection() const {
  Vector drag_start_ = drag_start;
  Vector drag_end = Vector(hovered_tile % 4, hovered_tile / 4);
  if (drag_start_.x > drag_end.x) {
    std::swap(drag_start_.x, drag_end.x);
  }
  if (drag_start_.y > drag_end.y) {
    std::swap(drag_start_.y, drag_end.y);
  }
  return Rectf(drag_start_, drag_end);
}

Rectf
EditorInputGui::selection_draw_rect() const {
  Rectf select = normalize_selection();
  select.p2 += Vector(1, 1);
  select.p1 = (select.p1 * 32) + Vector(Xpos, Ypos);
  select.p2 = (select.p2 * 32) + Vector(Xpos, Ypos);
  return select;
}

void
EditorInputGui::update_selection() {
  Rectf select = normalize_selection();
  tiles->tiles.clear();
  tiles->width = select.get_width() + 1;
  tiles->height = select.get_height() + 1;

  int size = active_tilegroup.size();
  for (int y = select.p1.y; y <= select.p2.y; y++) {
    for (int x = select.p1.x; x <= select.p2.x; x++) {
      int tile_pos = y*4 + x + starting_tile;
      if (tile_pos < size && tile_pos >= 0) {
        tiles->tiles.push_back(active_tilegroup[tile_pos]);
      } else {
        tiles->tiles.push_back(0);
      }
    }
  }
}

bool
EditorInputGui::event(SDL_Event& ev) {
  switch (ev.type) {
    case SDL_MOUSEBUTTONDOWN:
    {
      if(ev.button.button == SDL_BUTTON_LEFT || ev.button.button == SDL_BUTTON_RIGHT) {
        switch (hovered_item) {
          case HI_TILEGROUP:
          {
            auto editor = Editor::current();
            if(editor->tileset->tilegroups.size() > 1)
            {
              Editor::current()->disable_keyboard();
              MenuManager::instance().push_menu(MenuStorage::EDITOR_TILEGROUP_MENU);
            }
            else
            {
              active_tilegroup = editor->tileset->tilegroups[0].tiles;
              input_type = EditorInputGui::IP_TILE;
              reset_pos();
              update_mouse_icon();
            }
          }
            break;
          case HI_OBJECTS:
          {
            auto editor = Editor::current();
            if( (editor->get_worldmap_mode() && object_input->get_num_worldmap_groups() > 1) ||
                (!editor->get_worldmap_mode() && object_input->get_num_level_groups() > 1) )
            {
              Editor::current()->disable_keyboard();
              MenuManager::instance().push_menu(MenuStorage::EDITOR_OBJECTGROUP_MENU);
            }
            else
            {
              if(editor->get_worldmap_mode())
              {
                active_objectgroup = object_input->get_first_worldmap_group_index();
              }
              else
              {
                active_objectgroup = 0;
              }
              input_type = EditorInputGui::IP_OBJECT;
              reset_pos();
              update_mouse_icon();
            }
          }
            break;
          case HI_TILE:
            switch (input_type) {
              case IP_TILE: {
                dragging = true;
                drag_start = Vector(hovered_tile % 4, hovered_tile / 4);
                int size = active_tilegroup.size();
                int tile_pos = hovered_tile + starting_tile;
                if (tile_pos < size && tile_pos >= 0) {
                  tiles->set_tile(active_tilegroup[tile_pos]);
                } else {
                  tiles->set_tile(0);
                }
              } break;
              case IP_OBJECT: {
                int size = object_input->groups[active_objectgroup].icons.size();
                if (hovered_tile < size && hovered_tile >= 0) {
                  object = object_input->groups[active_objectgroup].icons[hovered_tile + starting_tile].object_name;
                }
                update_mouse_icon();
              } break;
              default:
                break;
            }
            return true;
            break;
          case HI_TOOL:
            switch (hovered_tile) {
              case 0:
                tiles->set_tile(0);
                object = "";
                update_mouse_icon();
                break;
              case 1:
                select_mode->next_mode();
                update_mouse_icon();
                break;
              case 2:
                move_mode->next_mode();
                update_mouse_icon();
                break;
              case 3:
                Editor::current()->esc_press();
                break;
              default:
                break;
            }
            return true;
            break;
          default:
            return false;
            break;
        }
      }
    } break;

    case SDL_MOUSEBUTTONUP:
      dragging = false;
      return false;
      break;

    case SDL_MOUSEMOTION:
    {
      Vector mouse_pos = VideoSystem::current()->get_renderer().to_logical(ev.motion.x, ev.motion.y);
      float x = mouse_pos.x - Xpos;
      float y = mouse_pos.y - Ypos;
      if (x < 0) {
        hovered_item = HI_NONE;
        tile_scrolling = TS_NONE;
        return false;
      }
      if (y < 0) {
        if (y < -38) {
          hovered_item = HI_TILEGROUP;
        } else if (y < -16) {
          hovered_item = HI_OBJECTS;
        } else {
          hovered_item = HI_TOOL;
          hovered_tile = get_tool_pos(mouse_pos);
        }
        tile_scrolling = TS_NONE;
        break;
      } else {
        hovered_item = HI_TILE;
        hovered_tile = get_tile_pos(mouse_pos);
        if (dragging && input_type == IP_TILE) {
          update_selection();
        }
      }
      if (y < 16) {
        tile_scrolling = TS_UP;
        using_scroll_wheel = false;
      }else if (y > SCREEN_HEIGHT - 16 - Ypos) {
        tile_scrolling = TS_DOWN;
        using_scroll_wheel = false;
      } else {
        tile_scrolling = TS_NONE;
      }
    }
    break;

    case SDL_WINDOWEVENT:
      if (ev.window.event == SDL_WINDOWEVENT_RESIZED) {
        resize();
      }
      return false;
    case SDL_MOUSEWHEEL:
    {
      if (hovered_item != HI_NONE)
      {
        if (ev.wheel.y > 0) {
          tile_scrolling = TS_UP;
        }
        else {
          tile_scrolling = TS_DOWN;
        }
        using_scroll_wheel = true;
        wheel_scroll_amount = ev.wheel.y;
      }
    }
    default:
      return false;
      break;
  }
  return true;
}

void
EditorInputGui::resize() {
  Xpos = SCREEN_WIDTH - 128;
  rubber->pos        = Vector(Xpos     , 44);
  select_mode->pos   = Vector(Xpos + 32, 44);
  move_mode->pos     = Vector(Xpos + 64, 44);
  settings_mode->pos = Vector(Xpos + 96, 44);
}

void
EditorInputGui::setup() {
  resize();
  tiles->set_tile(0);
}

void
EditorInputGui::reset_pos() {
  starting_tile = 0;
}

void
EditorInputGui::update_mouse_icon() {
  switch (input_type) {
    case IP_NONE:
      MouseCursor::current()->set_icon(NULL);
      break;
    case IP_OBJECT:
      if (object.empty()) {
        MouseCursor::current()->set_icon(rubber->get_current_surface());
      } else {
        MouseCursor::current()->set_icon(move_mode->get_current_surface());
      }
      break;
    case IP_TILE:
      MouseCursor::current()->set_icon(select_mode->get_current_surface());
      break;
    default:
      break;
  }
}

Vector
EditorInputGui::get_tile_coords(const int pos) const {
  int x = pos%4;
  int y = pos/4;
  return Vector( x * 32 + Xpos, y * 32 + Ypos);
}

int
EditorInputGui::get_tile_pos(const Vector& coords) const {
  int x = (coords.x - Xpos) / 32;
  int y = (coords.y - Ypos) / 32;
  return y*4 + x;
}

Vector
EditorInputGui::get_tool_coords(const int pos) const {
  int x = pos%4;
  int y = pos/4;
  return Vector( x * 32 + Xpos, y * 16 + 44);
}

int
EditorInputGui::get_tool_pos(const Vector& coords) const {
  int x = (coords.x - Xpos) / 32;
  int y = (coords.y - 44)   / 16;
  return y*4 + x;
}

/* EOF */
