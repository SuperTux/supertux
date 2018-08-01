//  SuperTux
//  Copyright (C) 2014 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_SUPERTUX_VIDEO_SDL_SDL_VIDEO_SYSTEM_HPP
#define HEADER_SUPERTUX_VIDEO_SDL_SDL_VIDEO_SYSTEM_HPP

#include <memory>
#include <SDL.h>

#include "math/size.hpp"
#include "video/video_system.hpp"
#include "video/viewport.hpp"

class SDLLightmap;
class SDLRenderer;
class TextureManager;

class SDLVideoSystem final : public VideoSystem
{
public:
  SDLVideoSystem();
  ~SDLVideoSystem();

  virtual Renderer& get_renderer() const override;
  virtual Lightmap& get_lightmap() const override;

  virtual TexturePtr new_texture(SDL_Surface* image) override;

  virtual const Viewport& get_viewport() const override { return m_viewport; }
  virtual void apply_config() override;
  virtual void flip() override;
  virtual void on_resize(int w, int h) override;

  virtual void set_gamma(float gamma) override;
  virtual void set_title(const std::string& title) override;
  virtual void set_icon(SDL_Surface* icon) override;

  virtual void do_take_screenshot() override;

private:
  void apply_video_mode();

private:
  SDL_Window* m_sdl_window;
  SDL_Renderer* m_sdl_renderer;
  Size m_desktop_size;
  Viewport m_viewport;
  std::unique_ptr<SDLRenderer> m_renderer;
  std::unique_ptr<SDLLightmap> m_lightmap;
  std::unique_ptr<TextureManager> m_texture_manager;

private:
  SDLVideoSystem(const SDLVideoSystem&) = delete;
  SDLVideoSystem& operator=(const SDLVideoSystem&) = delete;
};

#endif

/* EOF */
