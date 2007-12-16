//          Copyright Timothy Goya 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef UNISON_VFS_FILE_SYSTEM_HPP
#define UNISON_VFS_FILE_SYSTEM_HPP

#include <string>
#include <vector>

namespace Unison
{
   namespace VFS
   {
      class FileSystem
      {
         public:
            static FileSystem &get()
            {
               static FileSystem vfs;
               return vfs;
            }

            void follow_sym_links(bool follow);

            std::string get_dir_sep();
            std::string get_base_dir();
            std::string get_user_dir();

            std::string get_write_dir();
            void set_write_dir(const std::string &write_dir);

            void mount(const std::string &path, const std::string &mount_point = "/", bool append = false);
            void umount(const std::string &path);
            std::vector<std::string> get_search_path();
            std::string get_mount_point(const std::string &path);

            void mkdir(const std::string &dir);
            void rm(const std::string &filename);
            std::vector<std::string> ls(const std::string &path);
            bool exists(const std::string &filename);
            bool is_dir(const std::string &filename);

            std::string get_real_dir(const std::string &filename);
         private:
            FileSystem();
            ~FileSystem();
      };
   }
}

#endif
