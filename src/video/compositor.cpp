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
#include "video/drawing_request.hpp"
#include "video/lightmap.hpp"
#include "video/renderer.hpp"
#include "video/video_system.hpp"

bool Compositor::s_render_lighting = true;

Compositor::Compositor(VideoSystem& video_system) :
  m_video_system(video_system),
  m_obst(),
  m_drawing_contexts()
{
  obstack_init(&m_obst);
}

Compositor::~Compositor()
{
  m_drawing_contexts.clear();
  obstack_free(&m_obst, NULL);
}

DrawingContext&
Compositor::make_context(bool overlay)
{
  m_drawing_contexts.emplace_back(new DrawingContext(m_video_system, m_obst, overlay));
  return *m_drawing_contexts.back();
}

void
Compositor::render()
{
  auto& renderer = m_video_system.get_renderer();
  auto& lightmap = m_video_system.get_lightmap();

  bool use_lightmap = std::any_of(m_drawing_contexts.begin(), m_drawing_contexts.end(),
                                  [](std::unique_ptr<DrawingContext>& ctx){
                                    return ctx->use_lightmap();
                                  });

  use_lightmap = use_lightmap && s_render_lighting;

  // prepare lightmap
  if (use_lightmap)
  {
    lightmap.start_draw();

    for(auto& ctx : m_drawing_contexts)
    {
      if (!ctx->is_overlay())
      {
        lightmap.set_clip_rect(ctx->get_viewport());
        lightmap.clear(ctx->get_ambient_color());

        ctx->light().render(m_video_system, Canvas::ALL);

        lightmap.clear_clip_rect();
      }
    }
    lightmap.end_draw();
  }

  // compose the screen
  {
    renderer.start_draw();

    for(auto& ctx : m_drawing_contexts)
    {
      renderer.set_clip_rect(ctx->get_viewport());
      ctx->color().render(m_video_system, Canvas::BELOW_LIGHTMAP);
      renderer.clear_clip_rect();
    }

    if (use_lightmap)
    {
      lightmap.render();
    }

    // Render overlay elements
    for(auto& ctx : m_drawing_contexts)
    {
      renderer.set_clip_rect(ctx->get_viewport());
      ctx->color().render(m_video_system, Canvas::ABOVE_LIGHTMAP);
      renderer.clear_clip_rect();
    }

    renderer.end_draw();
  }

  // cleanup
  for(auto& ctx : m_drawing_contexts)
  {
    ctx->clear();
  }
  m_video_system.flip();

  obstack_free(&m_obst, NULL);
  obstack_init(&m_obst);
}

/* EOF */
