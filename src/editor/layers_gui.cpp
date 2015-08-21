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

#include "editor/layers_gui.hpp"

#include "editor/editor.hpp"
#include "editor/layer_icon.hpp"
#include "gui/menu_manager.hpp"
#include "math/vector.hpp"
#include "object/tilemap.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/menu/editor_tilegroup_menu.hpp"
#include "supertux/colorscheme.hpp"
#include "supertux/game_object.hpp"
#include "supertux/globals.hpp"
#include "supertux/level.hpp"
#include "supertux/resources.hpp"
#include "supertux/sector.hpp"
#include "util/gettext.hpp"
#include "video/drawing_context.hpp"
#include "video/font.hpp"
#include "video/renderer.hpp"
#include "video/video_system.hpp"

EditorLayersGui::EditorLayersGui() :
  layers(),
  selected_tilemap(),
  Ypos(448),
  Width(512),
  sector_text(),
  sector_text_width(0),
  hovered_item(HI_NONE),
  hovered_layer(-1)
{
}

EditorLayersGui::~EditorLayersGui()
{
}

void
EditorLayersGui::draw(DrawingContext& context) {
  context.draw_filled_rect(Rectf(Vector(0, Ypos), Vector(Width, SCREEN_HEIGHT)),
                           Color(0.9f, 0.9f, 1.0f, 0.6f),
                           0.0f,
                           LAYER_GUI-10);

  switch (hovered_item) {
    case HI_SPAWNPOINTS:
      context.draw_filled_rect(Rectf(Vector(0, Ypos), Vector(Xpos, SCREEN_HEIGHT)),
                               Color(0.9f, 0.9f, 1.0f, 0.6f),
                               0.0f,
                               LAYER_GUI-5);
      break;
    case HI_SECTOR:
      context.draw_filled_rect(Rectf(Vector(Xpos, Ypos), Vector(sector_text_width + Xpos, SCREEN_HEIGHT)),
                               Color(0.9f, 0.9f, 1.0f, 0.6f),
                               0.0f,
                               LAYER_GUI-5);
      break;
    case HI_LAYERS: {
      Vector coords = get_layer_coords(hovered_layer);
      context.draw_filled_rect(Rectf(coords, coords + Vector(32, 32)),
                               Color(0.9f, 0.9f, 1.0f, 0.6f),
                               0.0f,
                               LAYER_GUI-5);
    } break;
    default: break;
  }

  if (!Editor::current()->levelloaded) {
    return;
  }

  context.draw_text(Resources::normal_font, sector_text,
                    Vector(35, Ypos+5),
                    ALIGN_LEFT, LAYER_GUI, ColorScheme::Menu::default_color);

  int pos = -1;
  for(auto i = layers.begin(); i != layers.end(); ++i) {
    pos++;
    LayerIcon* li = &(**i);
    li->draw(context, get_layer_coords(pos));
  }

}

void
EditorLayersGui::update(float elapsed_time) {

}

bool
EditorLayersGui::event(SDL_Event& ev) {
  switch (ev.type) {
    case SDL_MOUSEBUTTONDOWN:
    {
      if(ev.button.button == SDL_BUTTON_LEFT)
      {
        switch (hovered_item) {
          case HI_SECTOR:
            Editor::current()->disable_keyboard();
            MenuManager::instance().set_menu(MenuStorage::EDITOR_SECTORS_MENU);
            break;
          case HI_LAYERS:
            if ( layers[hovered_layer]->is_tilemap ) {
              if (selected_tilemap) {
                ((TileMap*)selected_tilemap)->editor_active = false;
              }
              selected_tilemap = layers[hovered_layer]->layer;
              ((TileMap*)selected_tilemap)->editor_active = true;
            }
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
      if (y < 0 || x > Width) {
        hovered_item = HI_NONE;
        return false;
        break;
      }
      if (x < 0) {
        hovered_item = HI_SPAWNPOINTS;
        break;
      } else {
        if (x <= sector_text_width) {
          hovered_item = HI_SECTOR;
        } else {
          hovered_item = HI_LAYERS;
          hovered_layer = get_layer_pos(mouse_pos);
        }
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
EditorLayersGui::resize() {
  Ypos = SCREEN_HEIGHT - 32;
  Width = SCREEN_WIDTH - 128;
}

void
EditorLayersGui::setup() {
  resize();
}

void
EditorLayersGui::refresh_sector_text() {
  sector_text = _("Sector") + ": " + Editor::current()->currentsector->get_name();
  sector_text_width  = int(Resources::normal_font->get_text_width(sector_text)) + 6;
}

void
EditorLayersGui::add_layer(GameObject* layer, std::string pic) {
  std::unique_ptr<LayerIcon> icon(new LayerIcon(pic, layer));
  int z_pos = icon->get_zpos();

  // The icon is inserted to the correct position.
  for(auto i = layers.begin(); i != layers.end(); ++i) {
    LayerIcon* li = &(**i);
    if (li->get_zpos() < z_pos) {
      layers.insert(i,move(icon));
      return;
    }
  }
  layers.push_back(move(icon));
}

Vector
EditorLayersGui::get_layer_coords(const int pos){
  return Vector( pos * 35 + Xpos + sector_text_width, Ypos);
}

int
EditorLayersGui::get_layer_pos(const Vector coords){
  return (coords.x - Xpos - sector_text_width) / 35;
}

/* EOF */
