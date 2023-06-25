//  SuperTux
//  Copyright (C) 2016 Ingo Ruhnke <grumbel@gmail.com>
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

#include "video/sdl_surface.hpp"

#include <sstream>
#include <stdexcept>

#include <physfs.h>
#define STBI_WRITE_NO_STDIO
#include <stb_image_write.h>

#include "physfs/physfs_sdl.hpp"
#include "physfs/physfs_stbi.hpp"
#include "util/log.hpp"

SDLSurfacePtr
SDLSurface::create_rgba(int width, int height)
{
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
  Uint32 rmask = 0xff000000;
  Uint32 gmask = 0x00ff0000;
  Uint32 bmask = 0x0000ff00;
  Uint32 amask = 0x000000ff;
#else
  Uint32 rmask = 0x000000ff;
  Uint32 gmask = 0x0000ff00;
  Uint32 bmask = 0x00ff0000;
  Uint32 amask = 0xff000000;
#endif
  SDLSurfacePtr surface(SDL_CreateRGBSurface(0, width, height, 32, rmask, gmask, bmask, amask));
  if (!surface) {
    std::ostringstream out;
    out << "failed to create SDL_Surface: " << SDL_GetError();
    throw std::runtime_error(out.str());
  }

  return surface;
}

SDLSurfacePtr
SDLSurface::create_rgb(int width, int height)
{
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
  Uint32 rmask = 0xff000000;
  Uint32 gmask = 0x00ff0000;
  Uint32 bmask = 0x0000ff00;
  Uint32 amask = 0x00000000;
#else
  Uint32 rmask = 0x000000ff;
  Uint32 gmask = 0x0000ff00;
  Uint32 bmask = 0x00ff0000;
  Uint32 amask = 0x00000000;
#endif
  SDLSurfacePtr surface(SDL_CreateRGBSurface(0, width, height, 24, rmask, gmask, bmask, amask));
  if (!surface) {
   std::ostringstream out;
    out << "failed to create SDL_Surface: " << SDL_GetError();
    throw std::runtime_error(out.str());
  }

  return surface;
}

SDLSurfacePtr
SDLSurface::from_file(const std::string& filename)
{
  log_debug << "loading image: " << filename << std::endl;

  PHYSFS_File* file;
  std::unique_ptr<stbi_io_callbacks> callbacks = get_physfs_stbi_io_callbacks(filename, file);
  if (!callbacks)
  {
    std::ostringstream msg;
    msg << "Couldn't load image '" << filename << "' :" << PHYSFS_getLastErrorCode();
    throw std::runtime_error(msg.str());
  }

  int image_width, image_height, image_depth;
  unsigned char* image_data = stbi_load_from_callbacks(callbacks.get(), file, &image_width, &image_height, &image_depth, 0);
  PHYSFS_close(file);

  Uint32 rmask = 0, gmask = 0, bmask = 0, amask = 0;
  switch (image_depth)
  {
  case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
#endif
    amask = 0;
    break;

  case 2:
  {
    // Convert 16-bit gray+alpha image to RGBA image because SDL2 doesn't support it
    unsigned char* converted_image = reinterpret_cast<unsigned char*>(SDL_malloc(image_width*image_height*4));
    for(int i = 0; i<image_width*image_height; i++)
    {
      converted_image[i*4] = converted_image[i*4+1] = converted_image[i*4+2] = image_data[i*2];
      converted_image[i*4+3] = image_data[i*2+1];
    }
    stbi_image_free(image_data);
    image_data = converted_image;
    image_depth = 4;
    [[fallthrough]];
  }

  case 4:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif
    break;
  }

  SDL_Surface* surf = SDL_CreateRGBSurface(0, image_width, image_height, image_depth*8, rmask, gmask, bmask, amask);
  if(!surf)
  {
    std::ostringstream msg;
    msg << "Couldn't load image '" << filename << "' :" << SDL_GetError();
    throw std::runtime_error(msg.str());
  }
  else
  {
    SDL_LockSurface(surf);
    surf->pixels = image_data;
    SDL_UnlockSurface(surf);
    SDLSurfacePtr surface(surf);
    return surface;
  }
}

int
SDLSurface::save_png(const SDL_Surface& surface, const std::string& filename)
{
  PHYSFS_File* file = PHYSFS_openWrite(filename.c_str());
  if(!file)
  {
    log_warning << "Could not get SDLRWops for " << filename << ": " << PHYSFS_getLastErrorCode() << std::endl;
    return false;
  }
  const int ret = stbi_write_png_to_func(physfs_stbi_write_func, file, surface.w, surface.h, surface.format->BytesPerPixel, surface.pixels, surface.pitch);
  PHYSFS_close(file);
  if (!ret)
  {
    log_warning << "Saving " << filename << " failed";
    return false;
  }
  else
  {
    return true;
  }
}

/* EOF */
