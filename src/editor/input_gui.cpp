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
  object_groups(),
  hovered_item(HI_NONE),
  hovered_tile(-1),
  tile_scrolling(TS_NONE),
  starting_tile(0),
  Xpos(512)
{
  ObjectGroup og; //TODO: This must be done another way.

  og.name = _("Hostile");
  og.add_icon("angrystone",      "images/creatures/angrystone/charging-0.png");
  og.add_icon("bouncingsnowball","images/creatures/bouncing_snowball/left-5.png");
  og.add_icon("captainsnowball", "images/creatures/snowball/cpt-left-1.png");
  og.add_icon("crystallo",       "images/creatures/crystallo/crystallo-left-0.png");
  og.add_icon("fish",            "images/creatures/fish/down.png");
  og.add_icon("flame",           "images/creatures/flame/flame-0.png");
  og.add_icon("flyingsnowball",  "images/creatures/flying_snowball/left-0.png");
  og.add_icon("ghostflame",      "images/creatures/flame/ghostflame-0.png");
  og.add_icon("goldbomb",        "images/creatures/gold_bomb/left-0.png");
  og.add_icon("haywire",         "images/creatures/haywire/left-0.png");
  og.add_icon("iceflame",        "images/creatures/flame/iceflame.png");
  og.add_icon("igel",            "images/creatures/igel/walk-0.png");
  og.add_icon("jumpy",           "images/creatures/snowjumpy/left-down.png");
  og.add_icon("kugelblitz",      "images/creatures/kugelblitz/flying-0.png");
  og.add_icon("livefire",        "images/creatures/livefire/left-0.png");
  og.add_icon("mole",            "images/creatures/mole/mole-5.png");
  og.add_icon("mrbomb",          "images/creatures/mr_bomb/center.png");
  og.add_icon("mriceblock",      "images/creatures/mr_iceblock/left-0.png");
  og.add_icon("mrtree",          "images/creatures/mr_tree/walk-left-1.png");
  og.add_icon("owl",             "images/creatures/owl/left-0.png");
  og.add_icon("plant",           "images/creatures/plant/left-0.png");
  og.add_icon("poisonivy",       "images/creatures/poison_ivy/left-0.png");
  og.add_icon("short_fuse",      "images/creatures/short_fuse/center.png");
  og.add_icon("skullyhop",       "images/creatures/skullyhop/charging-0.png");
  og.add_icon("smartball",       "images/creatures/snowball/left-1.png");
  og.add_icon("snail",           "images/creatures/snail/snowsnail.png");
  og.add_icon("snowball",        "images/creatures/snowball/sport-left-1.png");
  og.add_icon("snowman",         "images/creatures/snowman/walk1.png");
  og.add_icon("spidermite",      "images/creatures/spidermite/spidermite0.png");
  og.add_icon("spiky",           "images/creatures/spiky/left-1.png");
  og.add_icon("sspiky",          "images/creatures/spiky/sleeping-left.png");
  og.add_icon("stumpy",          "images/creatures/mr_tree/small-left-0.png");
  og.add_icon("toad",            "images/creatures/toad/toad-idle-0.png");
  og.add_icon("totem",           "images/creatures/totem/walking1.png");
  og.add_icon("walkingleaf",     "images/creatures/walkingleaf/left-0.png");
  og.add_icon("willowisp",       "images/creatures/willowisp/idle-0.png");
  og.add_icon("zeekling",        "images/creatures/zeekling/left-0.png");
  object_groups.push_back(og);

  og.icons.clear();
  og.name = _("Projectiles");
  og.add_icon("dart",            "images/creatures/dart/flying.png");
  og.add_icon("kamikazesnowball","images/creatures/snowball/kamikaze-left.png");
  og.add_icon("mole_rock",       "images/creatures/mole/mole-rock.png");
  og.add_icon("skydive",         "images/creatures/skydive/skydive.png");
  og.add_icon("stalactite",      "images/creatures/stalactite/falling.png");
  og.add_icon("yeti_stalactite", "images/engine/editor/stalactite_yeti.png");
  object_groups.push_back(og);

  og.icons.clear();
  og.name = _("Bosses");
  og.add_icon("yeti",     "images/creatures/yeti/hudlife.png");
  og.add_icon("ghosttree","images/creatures/ghosttree/ghosttree.png");
  object_groups.push_back(og);

  og.icons.clear();
  og.name = _("Interactive");
  og.add_icon("bonusblock",        "images/objects/bonus_block/full-0.png");
  og.add_icon("heavycoin",         "images/objects/coin/heavy_coin.png");
  og.add_icon("coin",              "images/objects/coin/path_coin.png");
  og.add_icon("deacal",            "images/engine/editor/decal.png");
  og.add_icon("platform",          "images/objects/flying_platform/flying_platform-0.png");
  og.add_icon("pneumatic_platform","images/engine/editor/pneumaticplatform.png");
  og.add_icon("bicycle_platform",  "images/engine/editor/bicycleplatform.png");
  og.add_icon("powerup",           "images/engine/editor/powerup.png");
  og.add_icon("scriptedobject",    "images/engine/editor/scriptedobject.png");
  og.add_icon("firefly",           "images/objects/resetpoints/bell-m.png");
  og.add_icon("rock",              "images/objects/rock/rock.png");
  og.add_icon("rustytrampoline",   "images/objects/rusty-trampoline/trampoline1-0.png");
  og.add_icon("trampoline",        "images/objects/trampoline/trampoline1-0.png");
  og.add_icon("torch",             "images/objects/torch/torch1.png");
  og.add_icon("unstable_tile",     "images/objects/unstable_tile/snow-3.png");
  og.add_icon("weak_block",        "images/objects/weak_block/straw.png");
  object_groups.push_back(og);

  og.icons.clear();
  og.name = _("Lightmap");
  og.add_icon("candle",     "images/objects/candle/candle-1.png");
  og.add_icon("lantern",    "images/objects/lantern/lantern-1.png");
  og.add_icon("magicblock", "images/objects/magicblock/magicblock.png");
  og.add_icon("spotlight",  "images/objects/spotlight/spotlight_center.png");
  object_groups.push_back(og);

  og.icons.clear();
  og.name = _("Triggers");
  og.add_icon("door",           "images/objects/door/door-0.png");
  og.add_icon("pushbutton",     "images/objects/pushbutton/pushbutton-0.png");
  og.add_icon("switch",         "images/objects/switch/left-0.png");
  og.add_icon("climbable",      "images/engine/editor/climbable.png");
  og.add_icon("scripttrigger",  "images/engine/editor/scripttrigger.png");
  og.add_icon("secretarea",     "images/engine/editor/secretarea.png");
  og.add_icon("sequencetrigger","images/engine/editor/sequencetrigger.png");
  og.add_icon("ambient_sound",  "images/engine/editor/ambientsound.png");
  og.add_icon("wind",           "images/engine/editor/wind.png");
  object_groups.push_back(og);


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
    for(auto i = object_groups[active_objectgroup].icons.begin(); i != object_groups[active_objectgroup].icons.end(); ++i) {
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
        size = object_groups[active_objectgroup].icons.size();
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
            MenuManager::instance().set_menu(MenuStorage::EDITOR_TILEGROUP_MENU);
            break;
          case HI_OBJECTS:
            Editor::current()->disable_keyboard();
            MenuManager::instance().set_menu(MenuStorage::EDITOR_OBJECTGROUP_MENU);
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
EditorInputGui::setup() {
  Xpos = SCREEN_WIDTH - 128;
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
