//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//  Copyright (C) 2018 Ingo Ruhnke <grumbel@gmail.com>
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

#include "video/compositor.hpp"

#include "math/rect.hpp"
#include "video/drawing_context.hpp"
#include "video/renderer.hpp"
#include "video/video_system.hpp"

Compositor::Compositor(VideoSystem& video_system) :
  m_video_system(video_system),
  m_drawing_contexts()
{
}

DrawingContext&
Compositor::make_context()
{
  m_drawing_contexts.emplace_back(new DrawingContext(m_video_system));
  return *m_drawing_contexts.back();
}

void
Compositor::render()
{
  //Renderer& renderer = m_video_system.get_renderer();

  for(auto& ctx : m_drawing_contexts)
  {
    ctx->render();
  }

  m_video_system.flip();
}

/* EOF */
