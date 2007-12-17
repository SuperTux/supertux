//          Copyright Timothy Goya 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <unison/vfs/FileSystem.hpp>

#include <fstream>
#include <stdexcept>

#include <physfs.h>

namespace Unison
{
   namespace VFS
   {
      FileSystem::FileSystem()
      {
#if __USE_POSIX
         std::ifstream cmdline("/proc/self/cmdline");
         std::string argv0;
         std::getline(cmdline, argv0, '\0');
         int res = PHYSFS_init(argv0.c_str());
#else
         int res = PHYSFS_init(0);
#endif
         if(!res)
         {
            throw std::runtime_error("Failed to initialize PhysFS: " + std::string(PHYSFS_getLastError()));
         }
      }

      FileSystem::~FileSystem()
      {
         if(!PHYSFS_deinit())
         {
            throw std::runtime_error("Failed to deinitialize PhysFS: " + std::string(PHYSFS_getLastError()));
         }
      }

      void FileSystem::follow_sym_links(bool follow)
      {
         PHYSFS_permitSymbolicLinks(follow);
      }

      std::string FileSystem::get_dir_sep()
      {
         return PHYSFS_getDirSeparator();
      }

      std::string FileSystem::get_base_dir()
      {
         return PHYSFS_getBaseDir();
      }

      std::string FileSystem::get_user_dir()
      {
         return PHYSFS_getUserDir();
      }

      std::string FileSystem::get_write_dir()
      {
         return PHYSFS_getWriteDir();
      }

      void FileSystem::set_write_dir(const std::string &write_dir)
      {
         if(!PHYSFS_setWriteDir(write_dir.c_str()))
         {
            throw std::runtime_error("Failed to set write dir '" + write_dir + "': " + std::string(PHYSFS_getLastError()));
         }
      }

      void FileSystem::mount(const std::string &path, const std::string &mount_point, bool append)
      {
         if(!PHYSFS_mount(path.c_str(), mount_point.c_str(), append))
         {
            throw std::runtime_error("Failed to mount '" + path + "': " + std::string(PHYSFS_getLastError()));
         }
      }

      void FileSystem::umount(const std::string &path)
      {
         if(!PHYSFS_removeFromSearchPath(path.c_str()))
         {
            throw std::runtime_error("Failed to umount '" + path + "': " + std::string(PHYSFS_getLastError()));
         }
      }

      std::vector<std::string> FileSystem::get_search_path()
      {
         std::vector<std::string> paths;
         char **search_path = PHYSFS_getSearchPath();
         for(char **iter = search_path;*iter;++iter)
         {
            paths.push_back(*iter);
         }
         PHYSFS_freeList(search_path);
         return paths;
      }

      std::string FileSystem::get_mount_point(const std::string &path)
      {
         return PHYSFS_getMountPoint(path.c_str());
      }

      void FileSystem::mkdir(const std::string &dir)
      {
         if(!PHYSFS_mkdir(dir.c_str()))
         {
            throw std::runtime_error("Failed to mkdir '" + dir + "': " + std::string(PHYSFS_getLastError()));
         }
      }

      void FileSystem::rm(const std::string &filename)
      {
         if(!PHYSFS_delete(filename.c_str()))
         {
            throw std::runtime_error("Failed to delete '" + filename + "': " + std::string(PHYSFS_getLastError()));
         }
      }

      std::vector<std::string> FileSystem::ls(const std::string &path)
      {
         std::vector<std::string> files;
         char **physfs_files = PHYSFS_enumerateFiles(path.c_str());
         for(char **iter = physfs_files;*iter;++iter)
         {
            files.push_back(*iter);
         }
         PHYSFS_freeList(physfs_files);
         return files;
      }

      bool FileSystem::exists(const std::string &filename)
      {
         return PHYSFS_exists(filename.c_str());
      }

      bool FileSystem::is_dir(const std::string &filename)
      {
         return PHYSFS_isDirectory(filename.c_str());
      }

      std::string FileSystem::get_real_dir(const std::string &filename)
      {
         return PHYSFS_getRealDir(filename.c_str());
      }
   }
}
