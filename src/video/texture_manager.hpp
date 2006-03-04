#ifndef __IMAGE_TEXTURE_MANAGER_HPP__
#define __IMAGE_TEXTURE_MANAGER_HPP__

#include <string>
#include <vector>
#include <map>
#include <set>

class Texture;
class ImageTexture;

class TextureManager
{
public:
  TextureManager();
  ~TextureManager();

  ImageTexture* get(const std::string& filename);
  
  void register_texture(Texture* texture);
  void remove_texture(Texture* texture);

  void save_textures();
  void reload_textures();

private:
  friend class ImageTexture;
  void release(ImageTexture* texture);
  
  typedef std::map<std::string, ImageTexture*> ImageTextures;
  ImageTextures image_textures;

  ImageTexture* create_image_texture(const std::string& filename);

  typedef std::set<Texture*> Textures;
  Textures textures;

  struct SavedTexture
  {
    Texture* texture;
    int width;
    int height;
    char* pixels;
    int border;                              

    int min_filter;
    int mag_filter;
    int wrap_s;
    int wrap_t;
  };
  std::vector<SavedTexture> saved_textures;

  void save_texture(Texture* texture);
};

extern TextureManager* texture_manager;

#endif

