#include <config.h>

#include "image_texture.hpp"
#include "texture_manager.hpp"

ImageTexture::ImageTexture(SDL_Surface* surface)
  : Texture(surface, GL_RGBA), refcount(0)
{
}

ImageTexture::~ImageTexture()
{
}

void
ImageTexture::release()
{
  texture_manager->release(this);
}
