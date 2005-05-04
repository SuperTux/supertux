#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__

#include <set>
#include <string>

namespace FileSystem
{
  bool faccessible(const std::string& filename);
  bool fcreatedir(const std::string& relative_dir);
  bool fwriteable(const std::string& filename);
  std::set<std::string> read_directory(const std::string& pathname);
  std::set<std::string> dsubdirs(const std::string& rel_path,
          const std::string& expected_file);
  std::set<std::string> dfiles(const std::string& rel_path,
          const std::string& glob, const std::string& exception_str);
  std::string dirname(const std::string& filename);
}

#endif

