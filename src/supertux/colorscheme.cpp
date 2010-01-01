//  SuperTux
//  Copyright (C) 2009 qMax
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

#include "gui/menu.hpp"
#include "object/floating_text.hpp"
#include "object/level_time.hpp"
#include "object/text_object.hpp"
#include "supertux/levelintro.hpp"
#include "supertux/player_status.hpp"
#include "supertux/statistics.hpp"
#include "supertux/textscroller.hpp"
#include "trigger/climbable.hpp"
#include "trigger/secretarea_trigger.hpp"
#include "video/color.hpp"
#include "worldmap/worldmap.hpp"

Color LevelIntro::header_color(1.0,1.0,0.6);
Color LevelIntro::author_color(1.0,1.0,1.0);
Color LevelIntro::stat_hdr_color(0.2,0.5,1.0);
Color LevelIntro::stat_color(1.0,1.0,1.0);

Color Statistics::header_color(1.0,1.0,1.0);
Color Statistics::text_color(1.0,1.0,0.6);

Color Menu::default_color(1.0,1.0,1.0);
Color Menu::active_color(0.2,0.5,1.0);
Color Menu::inactive_color(0.5,0.5,0.5);
Color Menu::label_color(0.0,1.0,1.0);
Color Menu::field_color(1.0,1.0,0.6);

Color PlayerStatus::text_color(1.0,1.0,0.6);

Color TextObject::default_color(1.0,1.0,1.0);

Color FloatingText::text_color(1.0,1.0,0.6);

Color LevelTime::text_color(1.0,1.0,0.6);

Color SecretAreaTrigger::text_color(1.0,1.0,0.6);

Color Climbable::text_color(1.0,1.0,0.6);

Color worldmap::WorldMap::level_title_color(1.0,1.0,1.0);
Color worldmap::WorldMap::message_color(1.0,1.0,0.6);
Color worldmap::WorldMap::teleporter_message_color(1.0,1.0,1.0);

Color TextScroller::small_color(1.0,1.0,1.0);
Color TextScroller::heading_color(1.0,1.0,0.6);
Color TextScroller::reference_color(0.2,0.6,1.0);
Color TextScroller::normal_color(1.0,1.0,1.0);

/* EOF */
