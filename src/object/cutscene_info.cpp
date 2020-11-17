//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#include "object/cutscene_info.hpp"

#include <stdio.h>

#include "object/camera.hpp"
#include "supertux/resources.hpp"
#include "video/drawing_context.hpp"

CutsceneInfo::CutsceneInfo(/*const Vector& pos*/ const Camera& cam, const std::string& text_, const Level& parent) :
  position(cam.get_translation() + Vector(32, 32)),
  text(text_),
  camera(cam),
  level(parent)
{
}

void
CutsceneInfo::update(float dt_sec)
{
  position = camera.get_translation() + Vector(32, 32);
}

void
CutsceneInfo::draw(DrawingContext& context)
{
  if (level.m_is_in_cutscene && !level.m_skip_cutscene)
  {
    context.color().draw_text(Resources::normal_font, text, position, ALIGN_LEFT, LAYER_OBJECTS + 1000, CutsceneInfo::text_color);
  }
}

/* EOF */
