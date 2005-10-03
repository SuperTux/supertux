#ifndef __SURFACE_TEXTURE_HPP__
#define __SURFACE_TEXTURE_HPP__

#include <string>
#include "texture.hpp"

class ImageTexture : public Texture
{
private:
  std::string filename;
  float image_width;
  float image_height;
  int refcount;

public:
  float get_image_width() const
  {
    return image_width;
  }

  float get_image_height() const
  {
    return image_height;
  }
  
  float get_uv_right() const
  {
    return image_width / static_cast<float> (get_width());
  }
  
  float get_uv_bottom() const
  {
    return image_height / static_cast<float> (get_height());
  }

  void ref()
  {
    refcount++;
  }

  void unref()
  {
    assert(refcount > 0);
    refcount--;
    if(refcount == 0)
      release();
  }

private:
  friend class TextureManager;
  
  ImageTexture(SDL_Surface* surface);
  virtual ~ImageTexture();

  void release();
};

#endif

