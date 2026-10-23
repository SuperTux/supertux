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

#include "video/texture_manager.hpp"

#include <SDL3_image/SDL_image.h>
#include <assert.h>
#include <sstream>

#include <physfs.h>

#ifdef __vita__
#include <algorithm>
#include <atomic>
#include <cstdint>
#include <unordered_set>
#include <psp2/kernel/cpu.h>
#include <psp2/kernel/threadmgr.h>
#endif

#include "math/rect.hpp"
#include "physfs/physfs_sdl.hpp"
#ifdef __vita__
#include "physfs/vita_archive.hpp"
#endif
#include "util/file_system.hpp"
#include "util/log.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"
#include "video/color.hpp"
#include "video/gl.hpp"
#include "video/sampler.hpp"
#include "video/sdl_surface.hpp"
#include "video/texture.hpp"
#include "video/video_system.hpp"

namespace {

GLenum string2wrap(const std::string& text)
{
  if (text == "clamp-to-edge")
  {
    return GL_CLAMP_TO_EDGE;
  }
  else if (text == "repeat")
  {
    return GL_REPEAT;
  }
  else if (text == "mirrored-repeat")
  {
    return GL_MIRRORED_REPEAT;
  }
  else
  {
    log_warning << "unknown texture wrap: " << text << std::endl;
    return GL_CLAMP_TO_EDGE;
  }
}

GLenum string2filter(const std::string& text)
{
  if (text == "nearest")
  {
    return GL_NEAREST;
  }
  else if (text == "linear")
  {
    return GL_LINEAR;
  }
  else
  {
    log_warning << "unknown texture filter: " << text << std::endl;
    return GL_LINEAR;
  }
}

SDLSurfacePtr create_image_surface(const std::string& filename)
{
#ifdef __vita__
  try
  {
    return SDLSurface::from_file(filename);
  }
  catch (const std::exception&)
  {
  }
#else
  if (PHYSFS_exists(filename.c_str()))
    return SDLSurface::from_file(filename);
#endif

  // The image doesn't exist, so attempt to load a ".deprecated" version
  log_warning << "Image '" << filename << "' doesn't exist. Attempting to load \".deprecated\" version." << std::endl;
  return SDLSurface::from_file(FileSystem::strip_extension(filename) + ".deprecated" +
                               FileSystem::extension(filename));
}

} // namespace

#ifdef __vita__
struct TextureManager::PreloadJob
{
  std::vector<std::string> filenames;
  std::unordered_map<std::string, size_t> index;
  std::vector<SDLSurfacePtr> surfaces;
  std::unique_ptr<std::atomic<bool>[]> done;
  std::atomic<size_t> next;
  std::atomic<size_t> completed;
  std::vector<SceUID> threads;

  static SDLSurfacePtr load(const std::string& filename)
  {
    PHYSFS_File* file = PHYSFS_openRead(filename.c_str());
    if (!file)
      return SDLSurfacePtr();

    const PHYSFS_sint64 length = PHYSFS_fileLength(file);
    std::vector<Uint8> data(length > 0 ? static_cast<size_t>(length) : 0);
    const PHYSFS_sint64 got = length > 0 ? PHYSFS_readBytes(file, data.data(), length) : -1;
    PHYSFS_close(file);
    if (length <= 0 || got != length)
      return SDLSurfacePtr();

    return SDLSurfacePtr(IMG_Load_IO(SDL_IOFromConstMem(data.data(), static_cast<size_t>(length)), true));
  }

  void prefetch()
  {
    for (size_t i = 0; i < filenames.size(); ++i)
    {
      while (i > next + 200)
        sceKernelDelayThread(1000);
      vita_archive_prefetch(filenames[i]);
    }
  }

  void decode()
  {
    for (size_t i = next++; i < filenames.size(); i = next++)
    {
      surfaces[i] = load(filenames[i]);
      done[i] = true;
      completed++;
    }
  }
};

namespace {

struct PreloadArg
{
  TextureManager::PreloadJob* job;
  bool prefetch;
};

int preload_thread(SceSize, void* argp)
{
  PreloadArg* arg = static_cast<PreloadArg*>(argp);
  if (arg->prefetch)
    arg->job->prefetch();
  else
    arg->job->decode();
  return 0;
}

} // namespace
#endif

const std::string TextureManager::s_dummy_texture = "images/engine/missing.png";

TextureManager::TextureManager() :
  m_image_textures(),
  m_surfaces(),
#ifdef __vita__
  m_preload_job(),
  m_preloaded(),
#endif
  m_load_successful(false)
{
}

TextureManager::~TextureManager()
{
  for (const auto& texture : m_image_textures)
  {
    if (!texture.second.expired())
    {
      log_warning << "Texture '" << std::get<0>(texture.first) << "' not freed" << std::endl;
    }
  }
#ifdef __vita__
  finish_preload();
#endif
  m_image_textures.clear();
  m_surfaces.clear();
}

#ifdef __vita__
bool
TextureManager::preload(const std::vector<std::string>& filenames, bool wait)
{
  if (!wait && m_preload_job && m_preload_job->completed < m_preload_job->filenames.size())
    return false;
  finish_preload();

  std::vector<std::pair<PHYSFS_uint64, std::string>> sorted;
  std::unordered_set<std::string> seen;
  for (const auto& filename : filenames)
  {
    std::string normalized = FileSystem::normalize(filename);
    if (m_surfaces.count(normalized) || m_preloaded.count(normalized) || !seen.insert(normalized).second)
      continue;
    const PHYSFS_sint64 offset = vita_archive_offset(normalized);
    sorted.emplace_back(offset < 0 ? UINT64_MAX : static_cast<PHYSFS_uint64>(offset), std::move(normalized));
  }
  if (sorted.empty())
    return false;

  std::stable_sort(sorted.begin(), sorted.end(),
                   [] (const auto& a, const auto& b) { return a.first < b.first; });

  auto job = std::make_unique<PreloadJob>();
  for (auto& entry : sorted)
  {
    job->index[entry.second] = job->filenames.size();
    job->filenames.push_back(std::move(entry.second));
  }
  job->surfaces.resize(job->filenames.size());
  job->done.reset(new std::atomic<bool>[job->filenames.size()]);
  for (size_t i = 0; i < job->filenames.size(); ++i)
    job->done[i] = false;
  job->next = 0;
  job->completed = 0;

  auto start_thread = [&job] (bool prefetch, int cpu) {
    PreloadArg arg = { job.get(), prefetch };
    SceUID thread = sceKernelCreateThread("preload", preload_thread, 0x10000100, 256 * 1024, 0, cpu, nullptr);
    if (thread < 0)
      return;
    if (sceKernelStartThread(thread, sizeof(arg), &arg) < 0)
    {
      sceKernelDeleteThread(thread);
      return;
    }
    job->threads.push_back(thread);
  };

  start_thread(false, SCE_KERNEL_CPU_MASK_USER_0);
  start_thread(false, SCE_KERNEL_CPU_MASK_USER_1);
  start_thread(false, SCE_KERNEL_CPU_MASK_USER_2);
  if (job->threads.empty())
    return false;
  start_thread(true, SCE_KERNEL_CPU_MASK_USER_ALL);

  m_preload_job = std::move(job);
  return true;
}

void
TextureManager::finish_preload()
{
  if (!m_preload_job)
    return;

  for (SceUID thread : m_preload_job->threads)
  {
    sceKernelWaitThreadEnd(thread, nullptr, nullptr);
    sceKernelDeleteThread(thread);
  }
  for (size_t i = 0; i < m_preload_job->filenames.size(); ++i)
  {
    if (m_preload_job->surfaces[i])
      m_preloaded[m_preload_job->filenames[i]] = std::move(m_preload_job->surfaces[i]);
  }
  m_preload_job.reset();
}

void
TextureManager::discard_preloaded()
{
  finish_preload();
  m_preloaded.clear();
}
#endif

SDLSurfacePtr
TextureManager::load_surface(const std::string& filename)
{
#ifdef __vita__
  if (m_preload_job)
  {
    auto it = m_preload_job->index.find(filename);
    if (it != m_preload_job->index.end())
    {
      const size_t i = it->second;
      while (!m_preload_job->done[i])
        sceKernelDelayThread(100);
      if (m_preload_job->surfaces[i])
        return std::move(m_preload_job->surfaces[i]);
    }
  }

  auto it = m_preloaded.find(filename);
  if (it != m_preloaded.end())
  {
    SDLSurfacePtr surface = std::move(it->second);
    m_preloaded.erase(it);
    return surface;
  }
#endif
  return create_image_surface(filename);
}

TexturePtr
TextureManager::get(const ReaderMapping& mapping, const std::optional<Rect>& region)
{
  std::string filename;
  if (!mapping.get("file", filename))
  {
    log_warning << "'file' tag missing" << std::endl;
  }
  else
  {
    filename = FileSystem::join(mapping.get_doc().get_directory(), filename);
  }

  std::optional<Rect> rect;
  std::vector<int> rect_v;
  if (mapping.get("rect", rect_v))
  {
    if (rect_v.size() == 4)
    {
      rect = Rect(rect_v[0], rect_v[1], rect_v[2], rect_v[3]);
    }
    else
    {
      log_warning << "'rect' requires four elements" << std::endl;
    }
  }

  GLenum wrap_s = GL_CLAMP_TO_EDGE;
  GLenum wrap_t = GL_CLAMP_TO_EDGE;

  std::vector<std::string> wrap_v;
  if (mapping.get("wrap", wrap_v))
  {
    if (wrap_v.size() == 1)
    {
      wrap_s = string2wrap(wrap_v[0]);
      wrap_t = string2wrap(wrap_v[0]);
    }
    else if (wrap_v.size() == 2)
    {
      wrap_s = string2wrap(wrap_v[0]);
      wrap_t = string2wrap(wrap_v[1]);
    }
    else
    {
      log_warning << "unknown number of wrap arguments" << std::endl;
    }
  }

  GLenum filter = GL_LINEAR;
  std::string filter_s;
  if (mapping.get("filter", filter_s))
  {
    filter = string2filter(filter_s);
  }

  Vector animate(0.0f, 0.0f);
  std::vector<float> animate_v;
  if (mapping.get("animate", animate_v))
  {
    if (animate_v.size() == 2)
    {
      animate.x = animate_v[0];
      animate.y = animate_v[1];
    }
  }

  if (region)
  {
    if (!rect)
    {
      rect = region;
    }
    else
    {
      rect->left += region->left;
      rect->top += region->top;

      rect->right = rect->left + region->get_width();
      rect->bottom = rect->top + region->get_height();
    }
  }

  return get(filename, rect, Sampler(filter, wrap_s, wrap_t, animate));
}

TexturePtr
TextureManager::get(const std::string& _filename)
{
  std::string filename = FileSystem::normalize(_filename);
  Texture::Key key(filename, Rect(0, 0, 0, 0));
  auto i = m_image_textures.find(key);

  TexturePtr texture;
  if (i != m_image_textures.end())
    texture = i->second.lock();

  if (!texture) {
    texture = create_image_texture(filename, Sampler());
    texture->m_cache_key = key;
    m_image_textures[key] = texture;
  }

  return texture;
}

TexturePtr
TextureManager::get(const std::string& _filename,
                    const std::optional<Rect>& rect,
                    const Sampler& sampler)
{
  std::string filename = FileSystem::normalize(_filename);
  Texture::Key key = Texture::Key(filename, rect ? *rect : Rect());

  auto i = m_image_textures.find(key);

  TexturePtr texture;
  if (i != m_image_textures.end())
    texture = i->second.lock();

  if (!texture)
  {
    if (rect)
    {
      texture = create_image_texture(filename, *rect, sampler);
    }
    else
    {
      texture = create_image_texture(filename, sampler);
    }
    texture->m_cache_key = key;
    m_image_textures[key] = texture;
  }

  return texture;
}

void
TextureManager::reap_cache_entry(const Texture::Key& key)
{
  auto i = m_image_textures.find(key);
  if (i == m_image_textures.end())
  {
    log_warning << "no cache entry for '" << std::get<0>(key) << "'" << std::endl;
  }
  else
  {
    assert(i->second.expired());
    m_image_textures.erase(i);
  }
}

TexturePtr
TextureManager::create_image_texture(const std::string& filename, const Rect& rect, const Sampler& sampler)
{
  m_load_successful = true;
  try
  {
    SDLSurfacePtr surface = create_image_surface_raw(filename, rect, sampler);
    return VideoSystem::current()->new_texture(*surface, sampler);
  }
  catch(const std::exception& err)
  {
    log_warning << "Couldn't load texture '" << filename << "' (now using dummy texture): " << err.what() << std::endl;
    m_load_successful = false;
    return create_dummy_texture();
  }
}

const SDL_Surface&
TextureManager::get_surface(const std::string& filename)
{
  auto i = m_surfaces.find(filename);
  if (i != m_surfaces.end())
  {
    return *i->second;
  }

  SDLSurfacePtr surface = load_surface(filename);
  const SDL_PixelFormatDetails* format = SDL_GetPixelFormatDetails(surface.get()->format);
  if (format->Rmask == 0 &&
      format->Gmask == 0 &&
      format->Bmask == 0 &&
      format->Amask == 0)
  {
    SDL_Surface* surf;
    log_debug << "Wrong surface format for image " << filename << ". Compensating." << std::endl;
    surf = SDL_ConvertSurface(const_cast<SDL_Surface*>(surface.get()), SDL_PIXELFORMAT_RGBA8888);
    surface.reset(surf);
  }
  return *(m_surfaces[filename] = std::move(surface));
}

SDLSurfacePtr
TextureManager::create_image_surface_raw(const std::string& filename, const Rect& rect, const Sampler& sampler)
{
  assert(rect.valid());

  const SDL_Surface& src_surface = get_surface(filename);
  const SDL_PixelFormatDetails* format = SDL_GetPixelFormatDetails(src_surface.format);

  const SDL_Surface& surface = src_surface;

  SDLSurfacePtr subimage;
  if (!Rect(0, 0, surface.w, surface.h).contains(rect))
  {
    log_warning << filename << ": invalid subregion requested: image="
                << surface.w << "x" << surface.h << ", rect=" << rect << std::endl;

    subimage = SDLSurfacePtr(
      SDL_CreateSurface(rect.get_width(), rect.get_height(), src_surface.format)
    );

    Rect clipped_rect(std::max(0, rect.left),
                      std::max(0, rect.top),
                      std::min(subimage->w, rect.right),
                      std::min(subimage->w, rect.bottom));

    SDL_Rect srcrect = clipped_rect.to_sdl();
    SDL_BlitSurface(const_cast<SDL_Surface*>(&surface), &srcrect, subimage.get(), nullptr);
  }
  else
  {
    subimage = SDLSurfacePtr(SDL_CreateSurfaceFrom(
                              rect.get_width(), rect.get_height(),
                                src_surface.format,
                                static_cast<uint8_t*>(surface.pixels) +
                                  rect.top * surface.pitch +
                                  rect.left * format->bytes_per_pixel, surface.pitch));

    if (!subimage)
    {
      throw std::runtime_error("SDL_CreateSurfaceFrom() call failed");
    }
  }

  return subimage;
}

TexturePtr
TextureManager::create_image_texture(const std::string& filename, const Sampler& sampler)
{
  m_load_successful = true;
  try
  {
    SDLSurfacePtr surface = load_surface(filename);
    return VideoSystem::current()->new_texture(*surface, sampler);
  }
  catch (const std::exception& err)
  {
    log_warning << "Couldn't load texture '" << filename << "' (now using dummy texture): " << err.what() << std::endl;
    m_load_successful = false;
    return create_dummy_texture();
  }
}

SDLSurfacePtr
TextureManager::create_dummy_surface()
{
  // on error, try loading placeholder file
  try
  {
    SDLSurfacePtr surface = create_image_surface(s_dummy_texture);
    return surface;
  }
  catch (const std::exception& err)
  {
    // on error (when loading placeholder), try using empty surface,
    // when that fails to, just give up
    SDLSurfacePtr surface(SDL_CreateSurface(128, 128, SDL_GetPixelFormatForMasks(8, 0, 0, 0, 0)));
    if (!surface)
    {
      throw;
    }
    else
    {
      log_warning << "Couldn't load texture '" << s_dummy_texture << "' (now using empty one): " << err.what() << std::endl;
      return surface;
    }
  }
}

TexturePtr
TextureManager::create_dummy_texture() const
{
  SDLSurfacePtr surface = create_dummy_surface();
  return VideoSystem::current()->new_texture(*surface);
}

void
TextureManager::reload()
{
  // Reload surfaces
  for (auto& surface : m_surfaces)
  {
    SDLSurfacePtr surface_new;
    try
    {
      surface_new = create_image_surface(surface.first);
    }
    catch (const std::exception& err)
    {
      log_warning << "Couldn't load texture '" << surface.first << "' (now using dummy texture): " << err.what() << std::endl;
      surface_new = create_dummy_surface();
    }
    surface.second.reset(surface_new);
  }

  // Reload textures
  for (auto& texture : m_image_textures)
  {
    auto texture_ptr = texture.second.lock();

    SDLSurfacePtr surface;
    if (std::get<1>(texture.first).empty()) // No specific rect for texture
    {
      try
      {
        surface = create_image_surface(std::get<0>(texture.first));
      }
      catch (const std::exception& err)
      {
        log_warning << "Couldn't load texture '" << std::get<0>(texture.first) << "' (now using dummy texture): " << err.what() << std::endl;
        surface = create_dummy_surface();
      }
    }
    else // Texture has a specific rect
    {
      try
      {
        surface = create_image_surface_raw(std::get<0>(texture.first), std::get<1>(texture.first), texture_ptr->get_sampler());
      }
      catch (const std::exception& err)
      {
        log_warning << "Couldn't load texture '" << std::get<0>(texture.first) << "' (now using dummy texture): " << err.what() << std::endl;
        surface = create_dummy_surface();
      }
    }

    texture_ptr->reload(*surface);
  }
}

void
TextureManager::debug_print(std::ostream& out) const
{
  size_t total_texture_pixels = 0;
  out << "textures:begin" << std::endl;
  for(const auto& it : m_image_textures)
  {
    const auto& key = it.first;

    if (it.second.lock()) {
      total_texture_pixels += std::get<1>(key).get_area();
    }

    out << "  texture "
        << " filename:" << std::get<0>(key) << " " << std::get<1>(key)
        << " " << "use_count:" << it.second.use_count() << std::endl;
  }
  out << "textures:end" << std::endl;

  size_t total_surface_pixels = 0;
  out << "surfaces:begin" << std::endl;
  for(const auto& it : m_surfaces)
  {
    const auto& filename = it.first;
    const auto& surface = it.second;

    total_surface_pixels += surface->w * surface->h;
    out << "  surface filename:" << filename << " " << surface->w << "x" << surface->h << std::endl;
  }
  out << "surfaces:end" << std::endl;

  out << "total texture count:" << m_image_textures.size() << std::endl;
  out << "total texture pixels:" << total_texture_pixels << std::endl;

  out << "total surface count:" << m_surfaces.size() << std::endl;
  out << "total surface pixels:" << total_surface_pixels << std::endl;
}
