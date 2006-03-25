#include <config.h>

#include "texture_manager.hpp"

#include <assert.h>
#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include "physfs/physfs_sdl.hpp"
#include "image_texture.hpp"
#include "glutil.hpp"
#include "file_system.hpp"
#include "msg.hpp"

TextureManager* texture_manager = NULL;

TextureManager::TextureManager()
{
}

TextureManager::~TextureManager()
{
  for(ImageTextures::iterator i = image_textures.begin();
      i != image_textures.end(); ++i) {
    if(i->second == NULL)
      continue;
    msg_warning("Texture '" << i->first << "' not freed");
    delete i->second;
  }
}

ImageTexture*
TextureManager::get(const std::string& _filename)
{
  std::string filename = FileSystem::normalize(_filename);
  ImageTextures::iterator i = image_textures.find(filename);

  ImageTexture* texture = NULL;
  if(i != image_textures.end())
    texture = i->second;

  if(texture == NULL) {
    texture = create_image_texture(filename);
    image_textures[filename] = texture;
  }

  return texture;
}

void
TextureManager::release(ImageTexture* texture)
{
  image_textures[texture->filename] = NULL;
  delete texture;
}

void
TextureManager::register_texture(Texture* texture)
{
  textures.insert(texture);
}

void
TextureManager::remove_texture(Texture* texture)
{
  textures.erase(texture);
}

static inline int next_power_of_two(int val)
{
  int result = 1;
  while(result < val)
    result *= 2;
  return result;
}

ImageTexture*
TextureManager::create_image_texture(const std::string& filename)
{
  SDL_Surface* image = IMG_Load_RW(get_physfs_SDLRWops(filename), 1);
  if(image == NULL) {
    std::ostringstream msg;
    msg << "Couldn't load image '" << filename << "' :" << SDL_GetError();
    throw std::runtime_error(msg.str());
  }

  int texture_w = next_power_of_two(image->w);
  int texture_h = next_power_of_two(image->h);

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
  SDL_Surface* convert = SDL_CreateRGBSurface(SDL_SWSURFACE,
      texture_w, texture_h, 32,
      0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
#else
  SDL_Surface* convert = SDL_CreateRGBSurface(SDL_SWSURFACE,
      texture_w, texture_h, 32,
      0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
#endif

  if(convert == 0)
    throw std::runtime_error("Couldn't create texture: out of memory");

  SDL_SetAlpha(image, 0, 0);
  SDL_BlitSurface(image, 0, convert, 0);

  ImageTexture* result = NULL;
  try {
    result = new ImageTexture(convert);
    result->filename = filename;
    result->image_width = image->w;
    result->image_height = image->h;
  } catch(...) {
    delete result;
    SDL_FreeSurface(convert);
    throw;
  }
  
  SDL_FreeSurface(convert);
  return result;
}

void
TextureManager::save_textures()
{
  for(Textures::iterator i = textures.begin(); i != textures.end(); ++i) {
    save_texture(*i);
  }
  for(ImageTextures::iterator i = image_textures.begin();
      i != image_textures.end(); ++i) {
    save_texture(i->second);
  }
}

void
TextureManager::save_texture(Texture* texture)
{
  SavedTexture saved_texture;
  saved_texture.texture = texture;

  size_t pixelssize = saved_texture.width * saved_texture.height * 4;
  saved_texture.pixels = new char[pixelssize];

  saved_textures.push_back(saved_texture);
}

void
TextureManager::reload_textures()
{
  saved_textures.clear();
}

