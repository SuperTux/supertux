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

#include "editor/input_center.hpp"

#include "editor/editor.hpp"
#include "object/camera.hpp"
#include "object/tilemap.hpp"
#include "supertux/game_object.hpp"
#include "supertux/game_object_ptr.hpp"
#include "supertux/level.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"
#include "supertux/tile.hpp"
#include "supertux/tile_manager.hpp"
#include "supertux/tile_set.hpp"
#include "video/font.hpp"
#include "video/renderer.hpp"
#include "video/video_system.hpp"

EditorInputCenter::EditorInputCenter() :
  hovered_tile(0, 0),
  sector_pos(0, 0),
  mouse_pos(0, 0)
{
}

EditorInputCenter::~EditorInputCenter()
{
}

void
EditorInputCenter::event(SDL_Event& ev) {
  switch (ev.type) {
    case SDL_MOUSEBUTTONDOWN:
    {
      switch (Editor::current()->tileselect.input_type) {
        case EditorInputGui::IP_TILE: {
          //add tile

          if ( !Editor::current()->layerselect.selected_tilemap ) {
            return;
          }

          if ( hovered_tile.x < 0 || hovered_tile.y < 0 ||
               hovered_tile.x >= ((TileMap *)Editor::current()->layerselect.selected_tilemap)->get_width() ||
               hovered_tile.y >= ((TileMap *)Editor::current()->layerselect.selected_tilemap)->get_height()) {
            return;
          }

          ((TileMap *)Editor::current()->layerselect.selected_tilemap)->change(hovered_tile.x, hovered_tile.y,
                                                                               Editor::current()->tileselect.tile);
        } break;
        case EditorInputGui::IP_OBJECT:
          if (Editor::current()->tileselect.object != "") {
            //add object
            GameObjectPtr game_object;
            try {
              game_object = ObjectFactory::instance().create(Editor::current()->tileselect.object, sector_pos, LEFT);
            } catch(const std::exception& e) {
              log_warning << "Error creating object " << Editor::current()->tileselect.object << ": " << e.what() << std::endl;
              return;
            }
            if (game_object == NULL)
              throw std::runtime_error("Creating " + Editor::current()->tileselect.object + " object failed.");

            try {
              Editor::current()->currentsector->add_object(game_object);
            } catch(const std::exception& e) {
              log_warning << "Error adding object " << Editor::current()->tileselect.object << ": " << e.what() << std::endl;
              return;
            }
          }
        break;
        default:
          break;
      }
    } break;

    case SDL_MOUSEMOTION:
    {
      mouse_pos = VideoSystem::current()->get_renderer().to_logical(ev.motion.x, ev.motion.y);
      actualize_pos();
// update tip
    } break;
    default:
      break;
  }
}

void
EditorInputCenter::actualize_pos() {
  sector_pos = mouse_pos + Editor::current()->currentsector->camera->get_translation();
  hovered_tile = sp_to_tp(sector_pos);
}

void
EditorInputCenter::draw(DrawingContext& context) {
  if ( Editor::current()->tileselect.input_type == EditorInputGui::IP_TILE ) {

    if ( !Editor::current()->layerselect.selected_tilemap ) {
      return;
    }

    if ( Editor::current()->tileselect.tile == 0 || hovered_tile.x < 0 || hovered_tile.y < 0 ||
         hovered_tile.x >= ((TileMap *)Editor::current()->layerselect.selected_tilemap)->get_width() ||
         hovered_tile.y >= ((TileMap *)Editor::current()->layerselect.selected_tilemap)->get_height()) {
      return;
    }

    context.push_transform();
    context.set_alpha(0.5);

    const Tile* tg_tile = Editor::current()->level->tileset->get( Editor::current()->tileselect.tile );
    tg_tile->draw(context, tp_to_sp(hovered_tile) - Editor::current()->currentsector->camera->get_translation(),
                  LAYER_GUI-11);

    context.pop_transform();
  }
}

Vector
EditorInputCenter::tp_to_sp(Vector tp) {
  if (Editor::current()->layerselect.selected_tilemap) {
    Vector sp = Vector( tp.x * 32, tp.y * 32 );
    // This line is cool :DDDDD
    return sp + ((TileMap *)Editor::current()->layerselect.selected_tilemap)->get_offset();
  } else {
    return Vector(0, 0);
  }
}

Vector
EditorInputCenter::sp_to_tp(Vector sp) {
  if (Editor::current()->layerselect.selected_tilemap) {
    // This line is cool :DDDDD
    sp -= ((TileMap *)Editor::current()->layerselect.selected_tilemap)->get_offset();
    int x = sp.x / 32;
    int y = sp.y / 32;
    return Vector( x, y );
  } else {
    return Vector(0, 0);
  }
}

/* EOF */
