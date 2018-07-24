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

#include "supertux/colorscheme.hpp"

#include "object/floating_text.hpp"
#include "object/level_time.hpp"
#include "object/text_object.hpp"
#include "supertux/levelintro.hpp"
#include "supertux/player_status.hpp"
#include "supertux/textscroller.hpp"
#include "trigger/climbable.hpp"
#include "trigger/secretarea_trigger.hpp"
#include "worldmap/worldmap.hpp"

Color LevelIntro::header_color(1.f,1.f,0.6f);
Color LevelIntro::author_color(1.f,1.f,1.f);
Color LevelIntro::stat_hdr_color(0.2f,0.5f,1.f);
Color LevelIntro::stat_color(1.f,1.f,1.f);

Color Statistics::header_color(1.f,1.f,1.f);
Color Statistics::text_color(1.f,1.f,0.6f);

Color ColorScheme::Menu::default_color(1.f,1.f,1.f);
Color ColorScheme::Menu::active_color(0.2f,0.5f,1.f);
Color ColorScheme::Menu::inactive_color(0.5f,0.5f,0.5f);
Color ColorScheme::Menu::label_color(0.f,1.f,1.f);
Color ColorScheme::Menu::field_color(1.f,1.f,0.6f);

Color PlayerStatus::text_color(1.f,1.f,0.6f);

Color TextObject::default_color(1.f,1.f,1.f);

Color FloatingText::text_color(1.f,1.f,0.6f);

Color LevelTime::text_color(1.f,1.f,0.6f);

Color SecretAreaTrigger::text_color(1.f,1.f,0.6f);

Color Climbable::text_color(1.f,1.f,0.6f);

Color worldmap::WorldMap::level_title_color(1.f,1.f,1.f);
Color worldmap::WorldMap::message_color(1.f,1.f,0.6f);
Color worldmap::WorldMap::teleporter_message_color(1.f,1.f,1.f);

Color TextScroller::small_color(1.f,1.f,1.f);
Color TextScroller::heading_color(1.f,1.f,0.6f);
Color TextScroller::reference_color(0.2f,0.6f,1.f);
Color TextScroller::normal_color(1.f,1.f,1.f);

/* EOF */
